#pragma once

#include <QWidget>

class HeaderBar : public QWidget
{
    Q_OBJECT

public:
    explicit HeaderBar(QWidget* parent = nullptr);

signals:
    void openFolderRequested();
};