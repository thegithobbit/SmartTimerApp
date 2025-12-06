#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include "TimerManager.h"
#include "EditTimerDialog.h"

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
    void onDeleteSelected();
    void onEditSelected();

    void refreshTable();
    void updateEditButtonVisibility();

private:
    QTableWidget *timerTable;
    QPushButton *addButton;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *deleteButton;
    QPushButton *editButton;

    TimerManager *manager;

    QString formatTime(int totalSeconds) const;
};

#endif // MAINWINDOW_H
