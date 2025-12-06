#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QList>
#include "TimerManager.h"
#include "EditTimerDialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addTimer_clicked();
    void on_startAll_clicked();
    void on_stopAll_clicked();
    void deleteSelectedTimers();
    void toggleSelectedTimers();
    void startSelectedTimers();
    void stopSelectedTimers();
    void on_editSelected_clicked();

    void updateEditButtonVisibility();

private:
    TimerManager *manager;

    QTableWidget *timerTable;
    QPushButton *addButton;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *deleteButton;
    QPushButton *toggleButton;
    QPushButton *editButton;

    void updateTimerList(const QList<TimerEntry*>& timers);
    QList<int> getSelectedRows() const;
};

#endif // MAINWINDOW_H
