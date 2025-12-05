#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include "TimerManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Кнопки
    void on_addTimer_clicked();
    void on_startAll_clicked();
    void on_stopAll_clicked();
    void on_deleteTimer_clicked();
    void on_toggleTimer_clicked();

    // Оновлення таблиці
    void updateTimerList(const QList<TimerEntry*>& timers);

private:
    Ui::MainWindow *ui;

    // UI елементи
    QTableWidget *timerTable;
    QPushButton *addButton;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *deleteButton;
    QPushButton *toggleButton;

    // Менеджер таймерів
    TimerManager *manager;

    // Подія закриття (необов'язкова, але реалізувати можна)
    void closeEvent(QCloseEvent *event) override {}
};

#endif // MAINWINDOW_H
