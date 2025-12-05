#include "mainwindow.h"
#include "TimerManager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(nullptr)
    , manager(new TimerManager(this))
{
    // Створюємо UI вручну (варіант А, без ui файлу)
    this->resize(600, 400);
    timerTable = new QTableWidget(this);
    timerTable->setColumnCount(4);
    timerTable->setHorizontalHeaderLabels({"Назва", "Залишилось", "Статус", "Дії"});
    timerTable->setGeometry(10, 10, 580, 300);

    addButton = new QPushButton("Додати таймер", this);
    addButton->setGeometry(10, 320, 120, 30);

    startButton = new QPushButton("Старт усіх", this);
    startButton->setGeometry(140, 320, 100, 30);

    stopButton = new QPushButton("Стоп усіх", this);
    stopButton->setGeometry(250, 320, 100, 30);

    deleteButton = new QPushButton("Видалити обране", this);
    deleteButton->setGeometry(360, 320, 120, 30);

    toggleButton = new QPushButton("Старт/Стоп обране", this);
    toggleButton->setGeometry(490, 320, 100, 30);

    // Підключаємо сигнали до слотів
    connect(addButton, &QPushButton::clicked, this, &MainWindow::on_addTimer_clicked);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::on_startAll_clicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::on_stopAll_clicked);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::on_deleteTimer_clicked);
    connect(toggleButton, &QPushButton::clicked, this, &MainWindow::on_toggleTimer_clicked);

    // Сигнали від TimerManager
    connect(manager, &TimerManager::timerUpdated, this, [this](int id, int remaining, bool running){
        // Конвертуємо всі таймери в QList<TimerEntry*>
        QVector<TimerEntry> allTimers = manager->getAllTimers();
        QList<TimerEntry*> timerPtrs;
        for (auto &t : allTimers) timerPtrs.append(&t);
        updateTimerList(timerPtrs);
    });

    connect(manager, &TimerManager::timerFinished, this, [this](int id){
        QMessageBox::information(this, "Таймер", "Таймер завершився!");
        QVector<TimerEntry> allTimers = manager->getAllTimers();
        QList<TimerEntry*> timerPtrs;
        for (auto &t : allTimers) timerPtrs.append(&t);
        updateTimerList(timerPtrs);
    });

    // Початкове оновлення таблиці
    QVector<TimerEntry> allTimers = manager->getAllTimers();
    QList<TimerEntry*> timerPtrs;
    for (auto &t : allTimers) timerPtrs.append(&t);
    updateTimerList(timerPtrs);
}

MainWindow::~MainWindow()
{
    delete manager;
    delete timerTable;
    delete addButton;
    delete startButton;
    delete stopButton;
    delete deleteButton;
    delete toggleButton;
}

void MainWindow::on_addTimer_clicked()
{
    QString name = QInputDialog::getText(this, "Новий таймер", "Назва:");
    if (name.isEmpty()) return;

    int secs = QInputDialog::getInt(this, "Час", "Секунди:", 60, 1);
    manager->addTimer(name, secs);

    QVector<TimerEntry> allTimers = manager->getAllTimers();
    QList<TimerEntry*> timerPtrs;
    for (auto &t : allTimers) timerPtrs.append(&t);
    updateTimerList(timerPtrs);
}

void MainWindow::on_startAll_clicked()
{
    for (auto &t : manager->getAllTimers()) {
        manager->startTimer(t.id);
    }
}

void MainWindow::on_stopAll_clicked()
{
    for (auto &t : manager->getAllTimers()) {
        manager->pauseTimer(t.id);
    }
}

void MainWindow::on_deleteTimer_clicked()
{
    auto selectedItems = timerTable->selectedItems();
    if (selectedItems.isEmpty()) return;

    int row = timerTable->row(selectedItems.first());
    int id = manager->getAllTimers()[row].id;
    manager->removeTimer(id);

    QVector<TimerEntry> allTimers = manager->getAllTimers();
    QList<TimerEntry*> timerPtrs;
    for (auto &t : allTimers) timerPtrs.append(&t);
    updateTimerList(timerPtrs);
}

void MainWindow::on_toggleTimer_clicked()
{
    auto selectedItems = timerTable->selectedItems();
    if (selectedItems.isEmpty()) return;

    int row = timerTable->row(selectedItems.first());
    TimerEntry t = manager->getAllTimers()[row];
    if (t.running) manager->pauseTimer(t.id);
    else manager->startTimer(t.id);

    QVector<TimerEntry> allTimers = manager->getAllTimers();
    QList<TimerEntry*> timerPtrs;
    for (auto &t : allTimers) timerPtrs.append(&t);
    updateTimerList(timerPtrs);
}

void MainWindow::updateTimerList(const QList<TimerEntry*>& timers)
{
    timerTable->setRowCount(timers.size());
    for (int i = 0; i < timers.size(); ++i) {
        TimerEntry* t = timers[i];
        timerTable->setItem(i, 0, new QTableWidgetItem(t->name));
        timerTable->setItem(i, 1, new QTableWidgetItem(QString::number(t->remainingSeconds)));
        timerTable->setItem(i, 2, new QTableWidgetItem(t->running ? "Біжить" : "Пауза"));

        QPushButton *toggleBtn = new QPushButton("Старт/Стоп");
        QPushButton *deleteBtn = new QPushButton("Видалити");
        QWidget *actions = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actions);
        layout->addWidget(toggleBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(2, 2, 2, 2);
        actions->setLayout(layout);
        timerTable->setCellWidget(i, 3, actions);

        connect(toggleBtn, &QPushButton::clicked, this, [=]() {
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);

            QVector<TimerEntry> allTimers = manager->getAllTimers();
            QList<TimerEntry*> timerPtrs;
            for (auto &t : allTimers) timerPtrs.append(&t);
            updateTimerList(timerPtrs);
        });

        connect(deleteBtn, &QPushButton::clicked, this, [=]() {
            manager->removeTimer(t->id);
            QVector<TimerEntry> allTimers = manager->getAllTimers();
            QList<TimerEntry*> timerPtrs;
            for (auto &t : allTimers) timerPtrs.append(&t);
            updateTimerList(timerPtrs);
        });
    }
}
