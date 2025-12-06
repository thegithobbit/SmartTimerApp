#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QSet>
#include "TimerManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTimer();
    void onStartSelected();
    void onStopSelected();
    void onEditSelected();

    void refreshTimersTable();
    void updateEditButtonVisibility();

private:
    Ui::MainWindow *ui;       // Обов'язково має бути!
    TimerManager *manager;

    QString formatTime(int totalSeconds) const;
};

#endif // MAINWINDOW_H
