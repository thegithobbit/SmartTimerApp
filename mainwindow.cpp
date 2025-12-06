#include "mainwindow.h"
#include "TimerManager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(nullptr)
    , manager(new TimerManager(this))
{
    // Створюємо UI вручну
    this->resize(600, 400);

    timerTable = new QTableWidget(this);
    timerTable->setColumnCount(5);
    timerTable->setHorizontalHeaderLabels({"✓", "Назва", "Залишилось", "Статус", "Дії"});
    timerTable->setGeometry(10, 10, 580, 300);
    timerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timerTable->setSelectionMode(QAbstractItemView::NoSelection);
    timerTable->horizontalHeader()->setHighlightSections(false);
    timerTable->horizontalHeader()->setFocusPolicy(Qt::NoFocus);

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
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedTimers);
    connect(toggleButton, &QPushButton::clicked, this, &MainWindow::on_toggleTimer_clicked);

    // Сигнали від TimerManager
    connect(manager, &TimerManager::timerUpdated, this, [this](int, int, bool){
        updateTimerList(manager->getAllTimersPointers());
    });

    connect(manager, &TimerManager::timerFinished, this, [this](int){
        QMessageBox::information(this, "Таймер", "Таймер завершився!");
        updateTimerList(manager->getAllTimersPointers());
    });

    // Початкове оновлення таблиці
    updateTimerList(manager->getAllTimersPointers());
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

    updateTimerList(manager->getAllTimersPointers());
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
    int id = manager->getAllTimersPointers()[row]->id;
    manager->removeTimer(id);

    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::on_toggleTimer_clicked()
{
    auto selectedItems = timerTable->selectedItems();
    if (selectedItems.isEmpty()) return;

    int row = timerTable->row(selectedItems.first());
    TimerEntry* t = manager->getAllTimersPointers()[row];
    if (t->running) manager->pauseTimer(t->id);
    else manager->startTimer(t->id);

    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::updateTimerList(const QList<TimerEntry*>& timers)
{
    timerTable->setRowCount(timers.size());

    for (int i = 0; i < timers.size(); ++i)
    {
        TimerEntry* t = timers[i];

        // Колонка 0: CheckBox
        QCheckBox *check = new QCheckBox();
        QWidget *checkWidget = new QWidget();
        QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
        checkLayout->addWidget(check);
        checkLayout->setAlignment(Qt::AlignCenter);
        checkLayout->setContentsMargins(0,0,0,0);
        checkWidget->setLayout(checkLayout);
        timerTable->setCellWidget(i, 0, checkWidget);

        // Колонки 1-3: Назва, Залишилось, Статус
        QTableWidgetItem *nameItem = new QTableWidgetItem(t->name);
        QTableWidgetItem *remainingItem = new QTableWidgetItem(QString::number(t->remainingSeconds));
        QTableWidgetItem *statusItem = new QTableWidgetItem(t->running ? "Біжить" : "Пауза");

        nameItem->setFlags(Qt::NoItemFlags);
        remainingItem->setFlags(Qt::NoItemFlags);
        statusItem->setFlags(Qt::NoItemFlags);

        timerTable->setItem(i, 1, nameItem);
        timerTable->setItem(i, 2, remainingItem);
        timerTable->setItem(i, 3, statusItem);

        // Колонка 4: Дії (кнопки)
        QPushButton *toggleBtn = new QPushButton("Старт/Стоп");
        QPushButton *deleteBtn = new QPushButton("Видалити");
        QWidget *actions = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actions);
        layout->addWidget(toggleBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(2,2,2,2);
        actions->setLayout(layout);
        timerTable->setCellWidget(i, 4, actions);

        // Логіка кнопок
        connect(toggleBtn, &QPushButton::clicked, this, [=]() {
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);
            updateTimerList(manager->getAllTimersPointers());
        });

        connect(deleteBtn, &QPushButton::clicked, this, [=]() {
            manager->removeTimer(t->id);
            updateTimerList(manager->getAllTimersPointers());
        });
    }
}

void MainWindow::deleteSelectedTimers()
{
    for (int row = timerTable->rowCount() - 1; row >= 0; --row)
    {
        QWidget *w = timerTable->cellWidget(row, 0);
        if (!w) continue;

        QCheckBox *check = w->findChild<QCheckBox*>();
        if (check && check->isChecked())
        {
            int id = manager->getAllTimersPointers()[row]->id;
            manager->removeTimer(id);
        }
    }
    updateTimerList(manager->getAllTimersPointers());
}
