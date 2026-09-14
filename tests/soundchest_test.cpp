#include "analysis/sound_analyzer.h"
#include "audio/audio_file_reader.h"
#include "library/audio_library_scanner.h"
#include "library/file_exporter.h"
#include "library/folder.h"
#include "waveform/peaks_builder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QTemporaryDir>
#include <QTimer>

#include <cmath>
#include <cstdio>

namespace {

int failures = 0;

void check(bool condition, const char *what) {
    std::printf("%s %s\n", condition ? "ok  " : "FAIL", what);
    if (!condition)
        ++failures;
}

bool writeSineWav(const QString &path, int sampleRate, double seconds) {
    const int frameCount = static_cast<int>(sampleRate * seconds);

    const auto append32 = [](QByteArray &data, quint32 v) { data.append(reinterpret_cast<const char *>(&v), 4); };
    const auto append16 = [](QByteArray &data, quint16 v) { data.append(reinterpret_cast<const char *>(&v), 2); };

    QByteArray data;
    data.append("RIFF", 4);
    append32(data, 36 + frameCount * 2);
    data.append("WAVE", 4);
    data.append("fmt ", 4);
    append32(data, 16);
    append16(data, 1);
    append16(data, 1);
    append32(data, static_cast<quint32>(sampleRate));
    append32(data, static_cast<quint32>(sampleRate * 2));
    append16(data, 2);
    append16(data, 16);
    data.append("data", 4);
    append32(data, static_cast<quint32>(frameCount * 2));

    for (int i = 0; i < frameCount; ++i) {
        const auto sample = static_cast<qint16>(0.5 * 32767.0 * std::sin(2.0 * 3.14159265358979 * 440.0 * i / sampleRate));
        append16(data, static_cast<quint16>(sample));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    return file.write(data) == data.size();
}

} // namespace

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    QTemporaryDir temp;
    if (!temp.isValid()) {
        std::fprintf(stderr, "could not create temp dir\n");
        return 2;
    }

    const QString wavPath = temp.filePath(QStringLiteral("sine.wav"));
    check(writeSineWav(wavPath, 44100, 1.0), "write test wav");

    QFile bogus(temp.filePath(QStringLiteral("bogus.wav")));
    if (bogus.open(QIODevice::WriteOnly))
        bogus.write("this is not audio");
    bogus.close();

    // audio::AudioFileReader
    {
        audio::AudioFileReader reader;
        check(!reader.open(temp.filePath(QStringLiteral("missing.wav"))), "reader rejects missing file");
        check(reader.open(wavPath), "reader opens a wav");
        check(reader.sampleRate() == 44100, "reader reports sample rate");
        check(reader.channels() == 1, "reader reports channels");
        check(reader.frameCount() == 44100, "reader reports frame count");
        check(std::abs(reader.durationSec() - 1.0) < 0.001, "reader reports duration");
        check(reader.formatLabel() == QStringLiteral("WAV (16-bit)"), "reader labels the format");
    }

    // library::AudioLibraryScanner
    {
        check(!library::AudioLibraryScanner::scan(temp.filePath(QStringLiteral("nope"))), "scanner rejects missing folder");

        QDir(temp.path()).mkdir(QStringLiteral("sub"));
        check(writeSineWav(temp.filePath(QStringLiteral("sub/nested.wav")), 22050, 0.5), "write nested wav");

        const auto folder = library::AudioLibraryScanner::scan(temp.path());
        check(folder != nullptr, "scanner returns a folder");
        if (folder) {
            check(folder->root != nullptr, "scanner returns a root node");
            check(folder->fileCount() == 2, "scanner finds both wavs and skips bogus.wav");
            check(folder->root->children.size() == 1, "scanner builds the subdirectory");
            check(folder->root->files.size() == 1, "scanner puts the root wav at the root");
            if (!folder->root->files.isEmpty())
                check(folder->root->files.first().fileName == QStringLiteral("sine.wav"), "scanner records file names");
        }
    }

    // waveform::PeaksBuilder
    {
        waveform::PeaksBuilder builder;
        bool                   finished = false;

        QObject::connect(&builder, &waveform::PeaksBuilder::ready, &app, [&](const QString &filePath) {
            const waveform::Peaks peaks = builder.peaks(filePath);
            check(peaks.valid(), "peaks are valid");
            check(peaks.mins.size() == 64, "peaks have the requested resolution");
            check(peaks.maxs.size() == 64, "peaks maxs have the requested resolution");
            check(peaks.maxs.value(0) > 0.9f && peaks.maxs.value(0) <= 1.0f, "peaks are normalized to their peak");

            bool inRange = true;
            for (int i = 0; i < peaks.mins.size(); ++i) {
                if (peaks.mins[i] < -1.01f || peaks.mins[i] > 1.01f || peaks.maxs[i] < -1.01f || peaks.maxs[i] > 1.01f)
                    inRange = false;
            }
            check(inRange, "peaks stay inside [-1, 1]");
            finished = true;
            app.quit();
        });

        QTimer::singleShot(3000, &app, [&] {
            if (!finished)
                check(false, "peaks builder finished in time");
            app.quit();
        });

        builder.request(wavPath, 64);
        app.exec();
    }

    // library::copyFiles
    {
        const QString          exportDir = temp.filePath(QStringLiteral("export"));
        const QVector<QString> sources{wavPath};

        const library::ExportResult first = library::copyFiles(sources, exportDir, library::CollisionPolicy::Rename);
        check(first.copied == 1 && first.failed == 0, "exporter copies a file");
        check(QFile::exists(QDir(exportDir).filePath(QStringLiteral("sine.wav"))), "exporter keeps the original name");

        const library::ExportResult second = library::copyFiles(sources, exportDir, library::CollisionPolicy::Rename);
        check(second.copied == 1, "exporter copies a second time");
        check(QFile::exists(QDir(exportDir).filePath(QStringLiteral("sine (2).wav"))), "exporter renames collisions");

        const library::ExportResult skipped = library::copyFiles(sources, exportDir, library::CollisionPolicy::Skip);
        check(skipped.skipped == 1 && skipped.copied == 0, "exporter can skip collisions");

        const library::ExportResult overwritten =
            library::copyFiles(sources, exportDir, library::CollisionPolicy::Overwrite);
        check(overwritten.copied == 1 && overwritten.failed == 0, "exporter can overwrite collisions");

        const library::ExportResult missing =
            library::copyFiles({temp.filePath(QStringLiteral("ghost.wav"))}, exportDir, library::CollisionPolicy::Rename);
        check(missing.failed == 1 && missing.errors.size() == 1, "exporter reports missing sources");
    }

    // analysis::SoundAnalyzer
    {
        analysis::SoundAnalyzer analyzer;
        bool                    finished = false;

        QObject::connect(&analyzer, &analysis::SoundAnalyzer::ready, &app, [&](const QString &filePath) {
            const analysis::SoundStats stats = analyzer.stats(filePath);
            check(stats.valid, "analyzer returns valid stats");
            check(std::abs(stats.samplePeakDbfs - (-6.02)) < 0.5, "analyzer sample peak is near -6 dBFS");
            check(std::abs(stats.truePeakDbtp - (-6.02)) < 0.5, "analyzer true peak is near -6 dBTP");
            check(std::abs(stats.rmsDbfs - (-9.03)) < 0.3, "analyzer RMS is near -9 dBFS");
            check(std::abs(stats.lufs - (-9.72)) < 0.5, "analyzer LUFS is near -9.7 LUFS");
            finished = true;
            app.quit();
        });

        QTimer::singleShot(3000, &app, [&] {
            if (!finished)
                check(false, "analyzer finished in time");
            app.quit();
        });

        analyzer.request(wavPath);
        app.exec();
    }

    std::printf(failures == 0 ? "PASS\n" : "FAILED (%d)\n", failures);
    return failures == 0 ? 0 : 1;
}
