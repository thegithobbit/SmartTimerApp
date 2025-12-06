#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QSet>

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
    void on_deleteTimer_clicked();       // Видалити один вибраний через інтерфейс (не чекбокс)
    void on_toggleTimer_clicked();       // Старт/Стоп один вибраний через інтерфейс

    // Для роботи з чекбоксами
    void deleteSelectedTimers();
    void startSelectedTimers();
    void stopSelectedTimers();
    void toggleSelectedTimers();

    // Оновлення таблиці
    void updateTimerList(const QList<TimerEntry*>& timers);

private:
    Ui::MainWindow *ui;

    // UI елементи
    QTableWidget *timerTable;
    QPushButton *addButton;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *deleteButton;          // Видалити обране (через чекбокси)
    QPushButton *toggleButton;          // Старт/Стоп для обраних

    // Менеджер таймерів
    TimerManager *manager;

    // Стан: чи додана колонка "№"
    bool numberColumnAdded;

    // Допоміжні
    void ensureNumberColumnExists(); // додає колонку №, якщо треба
    int checkboxColumnIndex() const;
    int nameColumnIndex() const;
    int remainingColumnIndex() const;
    int statusColumnIndex() const;
    int actionsColumnIndex() const;
};

#endif // MAINWINDOW_H
