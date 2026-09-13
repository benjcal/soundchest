# SoundChest architecture

Read a folder, see its audio files, play one, see its waveform.

## Layers

```
src/
  main.cpp        boot + theme + wiring of the three controllers
  app/            controllers: decisions and cross-layer wiring
  audio/          the file reader and the sound output
  library/        the user's collection: scanning + metadata
  waveform/       waveform data: peaks + async build
  ui/             every Qt widget, wrapper model, delegate, dialog
```

Dependencies point downward only:

```
app  ->  ui, waveform, library, audio
ui   ->  library::AudioFile, waveform::Peaks   (data types only, never services)
waveform -> audio::AudioFileReader
library  -> audio::AudioFileReader
audio    -> (libsndfile, Qt Multimedia at playback)
```

`ui/` may include domain *data* types (`library/audio_file.h`, `waveform/peaks.h`)
but never domain *services* (`AudioFileReader`, `AudioLibraryScanner`, `PeaksBuilder`).
Only `main.cpp` wires controllers together; controllers never include each other.

## Glossary

| Name | Kind | One line |
|---|---|---|
| `library::AudioFile` | data | metadata for one file |
| `library::Folder`, `library::FolderNode` | data | scan result tree |
| `library::AudioLibraryScanner` | service | walks a folder into a `Folder` |
| `audio::AudioFileReader` | service | opened file: metadata + frames; the only libsndfile user |
| `audio::Player` | service | playback via QMediaPlayer; async load, loop, volume |
| `waveform::Peaks` | data | one min/max pair per resolution column |
| `waveform::PeaksBuilder` | service | builds and caches `Peaks` on worker threads |
| `ui::Window` | widget | composes all widgets; the only window |
| `ui::FolderTreeWidget` | widget | shows the folder tree |
| `ui::FileTableWidget` | widget | shows the files, emits selected row |
| `ui::WaveformWidget` | widget | the big waveform |
| `ui::TransportControls` | widget | play, stop, loop, autoplay, volume |
| `ui::HeaderBar` | widget | title + open folder button |
| `ui::FolderTreeWidgetModel` | wrapper model | wraps a `Folder` for `FolderTreeWidget` |
| `ui::FileTableWidgetModel` | wrapper model | wraps `AudioFile`s for `FileTableWidget` |
| `ui::waveform_renderer` | functions | min/max waveform drawing for `WaveformWidget` |
| `app::LibraryController` | controller | scan, folder selection, file selection |
| `app::WaveformController` | controller | peaks for the big waveform view |
| `app::PlaybackController` | controller | player, transport, progress |

## Naming law

- `ui/` contains Qt glue only: widgets, wrapper models, delegates, dialogs.
- A wrapper model is named after the widget it serves: `<WidgetName>WidgetModel`.
  It holds data only; controllers push data into it.
- Domain services carry their verb: `Scanner`, `Reader`, `Builder`, `Player`.
- Domain data is a plain noun: `AudioFile`, `Folder`, `Peaks`.
- One word per concept: a "sound" is an `AudioFile`; it is never called a track,
  a file, or audio metadata in code.
- `ui::Window` is the one widget without a `Widget` suffix.

## Contracts

`audio::AudioFileReader`
- `open()` closes any previous file; returns false and sets `errorString()` on failure.
- `readFrames()` returns frames read, 0 at end of file; interleaved float samples.
- `seek()` returns the new frame index or -1.
- The only file in the project that includes `<sndfile.h>`.

`library::AudioLibraryScanner::scan()`
- Returns `nullptr` and sets the error string when the root does not exist or the
  tree is inconsistent.
- Files whose format cannot be read are skipped silently.
- Never throws; never touches the audio device or the UI.

`waveform::PeaksBuilder`
- `request(path, columns)` is idempotent. It starts a build unless an equal or
  finer resolution is already cached or pending.
- `peaks(path)` may be empty or coarser than requested; check `valid()`.
- `ready(path)` is emitted on the GUI thread after the cache was updated.
- `clear()` invalidates every pending build and empties the cache.
- The cache is accessed from the GUI thread only.

`app::LibraryController` signals
- `soundsShown()` after a directory's files land in the table model.
- `soundSelected(AudioFile)` after a row is selected.

## Threading

- GUI thread: everything except `buildPeaks`.
- `PeaksBuilder` owns a `QThreadPool` (max 4) and a generation counter.
  Worker results are posted back with `QMetaObject::invokeMethod(..., QueuedConnection)`.
- `Player` decodes and outputs on Qt Multimedia's own threads; only its signals
  (`loadFailed`) reach the GUI thread.

## Errors

- Scan failure: `Window::showError` with the scanner's error string.
- File open failure at playback: `Window::showError` with the player's error string;
  QMediaPlayer reports load errors asynchronously through `Player::loadFailed`.
- A file that cannot be read during a scan is skipped (it simply does not appear).
- A waveform that cannot be built is not an error: no peaks, no message.

## Data flow

```
HeaderBar -> LibraryController::openFolder -> AudioLibraryScanner
LibraryController -> FolderTreeWidgetModel, FileTableWidgetModel
LibraryController::soundSelected -> PlaybackController (opens player)
                                 -> WaveformController (requests peaks)
PeaksBuilder::ready -> WaveformController (big view)
TransportControls -> PlaybackController -> Player
PlaybackController -> Window::setProgress (50 ms timer)
```

## Adding things

- New file columns: `ui/file_table_widget_model.cpp` (`data`, `headerData`).
- New transport action: `TransportControls` signal + `PlaybackController` connection.
- Different waveform look: `ui/waveform_renderer.cpp` (the big view uses it).
- New scan metadata: `library/audio_library_scanner.cpp` reads it from `AudioFileReader`.
- New async work: a service in its own folder with a generation counter, owned in `main.cpp`.

## Tests

- `soundchest_test`: reader, scanner, peaks builder (no UI).
- `controller_flow`: boots the window, scans a folder, checks the table and peaks.

Run: `./build/soundchest_test` and `QT_QPA_PLATFORM=offscreen ./build/controller_flow <folder>`.
