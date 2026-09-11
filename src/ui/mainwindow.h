#pragma once

#include <QMainWindow>

namespace ui {

class MainView;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

    MainView *mainView() const;

  private:
    MainView *m_view;
};

} // namespace ui
