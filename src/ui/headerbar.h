#pragma once

#include <QWidget>

namespace ui {

class HeaderBar : public QWidget {
    Q_OBJECT

  public:
    explicit HeaderBar(QWidget *parent = nullptr);

  signals:
    void openFolderRequested();
    void aboutRequested();
};

} // namespace ui
