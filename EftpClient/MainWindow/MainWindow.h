#ifndef COMPUTER_TEST_H
#define COMPUTER_TEST_H

#include <QMainWindow>
#include "MainWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

private:
    Ui::MainWindow ui;
};
#endif // COMPUTER_TEST_H
