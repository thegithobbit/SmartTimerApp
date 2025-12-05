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
    // Створюємо таблицю і кнопки вручну
    timerTable = new QTableWidget(this);
    timerTable->setColumnCount(4);
    timerTable->setHorizontalHeaderLabels({"Назва", "Залишилось", "Статус", "Дії"});
    setCentralWidget(timerTable);

    addButton = new QPushButton("Додати", this);
    startButton = new QPushButton("Старт усіх", this);
    stopButton = new QPushButton("Стоп усіх", this);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::on_addTimer_clicked);
    connect(startButton, &QPushButton::clicked, this, [=]() {
        for (auto t : manager->getAllTimers())
            manager->startTimer(t.id);
    });
    connect(stopButton, &QPushButton::clicked, this, [=]() {
        for (auto t : manager->getAllTimers())
            manager->pauseTimer(t.id);
    });

    // Тут використовуємо слот updateTimerList (як у хедері)
    connect(manager, &TimerManager::timerUpdated, this, [=](int id, int remaining, bool running){
        Q_UNUSED(id)
        Q_UNUSED(remaining)
        Q_UNUSED(running)
        updateTimerList(manager->getAllTimers());
    });

    // Сигнал закінчення таймера
    connect(manager, &TimerManager::timerFinished, this, [=](int id){
        Q_UNUSED(id)
        QMessageBox::information(this, "Таймер", "Таймер завершився!");
    });
}

// --- Слоти ---
void MainWindow::on_addTimer_clicked()
{
    QString name = QInputDialog::getText(this, "Новий таймер", "Назва:");
    if (name.isEmpty()) return;

    int secs = QInputDialog::getInt(this, "Час", "Секунди:", 60, 1);
    manager->addTimer(name, secs);

    // Оновлюємо таблицю через updateTimerList
    updateTimerList(manager->getAllTimers());
}

// --- Оновлення таблиці ---
void MainWindow::updateTimerList(const QList<TimerEntry *> &timers)
{
    timerTable->setRowCount(timers.size());

    for (int row = 0; row < timers.size(); ++row) {
        TimerEntry* t = timers[row];
        timerTable->setItem(row, 0, new QTableWidgetItem(t->name));
        timerTable->setItem(row, 1, new QTableWidgetItem(QString::number(t->remainingSeconds)));
        timerTable->setItem(row, 2, new QTableWidgetItem(t->running ? "Біжить" : "Пауза"));

        QPushButton *startPauseBtn = new QPushButton(t->running ? "Пауза" : "Старт");
        QPushButton *editBtn = new QPushButton("Редагувати");
        QPushButton *deleteBtn = new QPushButton("Видалити");

        QWidget *actions = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actions);
        layout->addWidget(startPauseBtn);
        layout->addWidget(editBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(2,2,2,2);
        actions->setLayout(layout);

        timerTable->setCellWidget(row, 3, actions);

        connect(startPauseBtn, &QPushButton::clicked, this, [=](){
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);
        });

        connect(editBtn, &QPushButton::clicked, this, [=](){
            QString newName = QInputDialog::getText(this, "Редагувати", "Назва:", QLineEdit::Normal, t->name);
            if (newName.isEmpty()) return;
            int newSecs = QInputDialog::getInt(this, "Час", "Секунди:", t->durationSeconds, 1);
            manager->updateTimer(t->id, newName, newSecs);
            updateTimerList(manager->getAllTimers());
        });

        connect(deleteBtn, &QPushButton::clicked, this, [=](){
            manager->removeTimer(t->id);
            updateTimerList(manager->getAllTimers());
        });
    }
}
