#pragma once

#include <QObject>

namespace ui {
class FileTableWidget;
class Window;
} // namespace ui

namespace app {

class ExportController : public QObject {
    Q_OBJECT

  public:
    ExportController(ui::Window *window, ui::FileTableWidget *fileTable, QObject *parent = nullptr);

  private slots:
    void exportSelection();

  private:
    ui::Window          *m_window;
    ui::FileTableWidget *m_fileTable;
};

} // namespace app
