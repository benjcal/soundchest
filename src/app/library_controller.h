#pragma once

#include <QObject>

#include "library/audio_file.h"

class QModelIndex;

namespace ui {
class FileTableWidgetModel;
class FolderTreeWidgetModel;
class Window;
} // namespace ui

namespace app {

class LibraryController : public QObject {
    Q_OBJECT

  public:
    LibraryController(ui::Window *window, ui::FolderTreeWidgetModel *folderModel, ui::FileTableWidgetModel *fileModel,
                      QObject *parent = nullptr);

    void openFolder(const QString &path);

  signals:
    void soundsShown();
    void soundSelected(const library::AudioFile &file);

  private slots:
    void selectDirectory(const QModelIndex &index);
    void selectFile(int row);

  private:
    ui::Window                *m_window;
    ui::FolderTreeWidgetModel *m_folderModel;
    ui::FileTableWidgetModel  *m_fileModel;
};

} // namespace app
