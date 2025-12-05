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
    // Створення таблиці
    timerTable = new QTableWidget(this);
    timerTable->setColumnCount(4);
    timerTable->setHorizontalHeaderLabels({"Назва", "Залишилось", "Статус", "Дії"});
    setCentralWidget(timerTable);

    // Підключення сигналів від TimerManager
    connect(manager, &TimerManager::timerUpdated,
            this, [=](int id, int remaining, bool running){
                updateTimerList(manager->getAllTimers());
            });

    connect(manager, &TimerManager::timerFinished,
            this, [=](int id){
                QMessageBox::information(this, "Таймер", "Таймер завершився!");
                updateTimerList(manager->getAllTimers());
            });

    // Додати кнопку додавання таймера
    addButton = new QPushButton("Додати таймер", this);
    addButton->move(10, 10);
    addButton->show();
    connect(addButton, &QPushButton::clicked, this, [=]() {
        QString name = QInputDialog::getText(this, "Новий таймер", "Назва:");
        if (name.isEmpty()) return;

        int secs = QInputDialog::getInt(this, "Час", "Секунди:", 60, 1);
        int id = manager->addTimer(name, secs);

        updateTimerList(manager->getAllTimers());
    });
}

MainWindow::~MainWindow()
{
    delete manager;
    delete timerTable;
    delete addButton;
}

void MainWindow::updateTimerList(const QList<TimerEntry *> &timers)
{
    timerTable->setRowCount(0);
    for (auto t : timers) {
        int row = timerTable->rowCount();
        timerTable->insertRow(row);

        timerTable->setItem(row, 0, new QTableWidgetItem(t->name));
        timerTable->setItem(row, 1, new QTableWidgetItem(QString::number(t->remainingSeconds)));
        timerTable->setItem(row, 2, new QTableWidgetItem(t->running ? "Біжить" : "Пауза"));

        // Кнопки дій
        QPushButton *startPauseBtn = new QPushButton(t->running ? "Пауза" : "Старт");
        QPushButton *deleteBtn = new QPushButton("Видалити");

        QWidget *actions = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actions);
        layout->addWidget(startPauseBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(2, 2, 2, 2);
        actions->setLayout(layout);
        timerTable->setCellWidget(row, 3, actions);

        // Підключення кнопок
        connect(startPauseBtn, &QPushButton::clicked, this, [=]() {
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);
        });

        connect(deleteBtn, &QPushButton::clicked, this, [=]() {
            manager->removeTimer(t->id);
            updateTimerList(manager->getAllTimers());
        });
    }
}
