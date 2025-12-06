#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EditTimerDialog.h"
#include <QCheckBox>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), manager(new TimerManager(this))
{
    ui->setupUi(this);

    // Налаштування таблиці
    ui->timersTable->setColumnCount(6); // Галочка, №, Назва, Час, Старт/Стоп, Видалити
    ui->timersTable->setHorizontalHeaderLabels({"", "№", "Назва", "Час", "Старт/Стоп", "Видалити"});
    ui->timersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->timersTable->verticalHeader()->setVisible(false);

    // Кнопки знизу
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddTimer);
    connect(ui->startSelectedButton, &QPushButton::clicked, this, &MainWindow::onStartSelected);
    connect(ui->stopSelectedButton, &QPushButton::clicked, this, &MainWindow::onStopSelected);
    connect(ui->deleteSelectedButton, &QPushButton::clicked, this, &MainWindow::onDeleteSelected);
    connect(ui->editButton, &QPushButton::clicked, this, &MainWindow::onEditSelected);

    // Сигнали від менеджера
    connect(manager, &TimerManager::timerUpdated, this, &MainWindow::refreshTimersTable);
    connect(manager, &TimerManager::timerFinished, this, &MainWindow::refreshTimersTable);

    refreshTimersTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::formatTime(int totalSeconds) const
{
    int h = totalSeconds / 3600;
    int m = (totalSeconds % 3600) / 60;
    int s = totalSeconds % 60;
    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

QSet<int> MainWindow::getSelectedTimerIds() const
{
    QSet<int> ids;
    for (int row = 0; row < ui->timersTable->rowCount(); ++row) {
        QCheckBox* cb = qobject_cast<QCheckBox*>(ui->timersTable->cellWidget(row, 0));
        if(cb && cb->isChecked()) {
            int id = ui->timersTable->item(row, 1)->data(Qt::UserRole).toInt();
            ids.insert(id);
        }
    }
    return ids;
}

void MainWindow::refreshTimersTable()
{
    QSet<int> selectedIds = getSelectedTimerIds();
    ui->timersTable->setRowCount(0);
    auto timers = manager->getAllTimersPointers();

    for (int i = 0; i < timers.size(); ++i) {
        TimerEntry* t = timers[i];
        ui->timersTable->insertRow(i);

        // Галочка
        QCheckBox* cb = new QCheckBox();
        cb->setChecked(selectedIds.contains(t->id));
        ui->timersTable->setCellWidget(i, 0, cb);
        connect(cb, &QCheckBox::stateChanged, this, &MainWindow::updateEditButtonVisibility);

        // №
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(i+1));
        numItem->setData(Qt::UserRole, t->id);
        ui->timersTable->setItem(i, 1, numItem);

        // Назва
        ui->timersTable->setItem(i, 2, new QTableWidgetItem(t->name));

        // Час
        ui->timersTable->setItem(i, 3, new QTableWidgetItem(formatTime(t->remainingSeconds)));

        // Старт/Стоп кнопка
        QPushButton* startStopBtn = new QPushButton(t->running ? "Стоп" : "Старт");
        ui->timersTable->setCellWidget(i, 4, startStopBtn);
        connect(startStopBtn, &QPushButton::clicked, this, [this, t](){
            if(t->running) handleRowStop(t->id);
            else handleRowStart(t->id);
        });

        // Видалити рядок
        QPushButton* deleteBtn = new QPushButton("Видалити");
        ui->timersTable->setCellWidget(i, 5, deleteBtn);
        connect(deleteBtn, &QPushButton::clicked, this, [this, t](){
            handleRowDelete(t->id);
        });
    }

    updateEditButtonVisibility();
}

void MainWindow::updateEditButtonVisibility()
{
    int selectedCount = getSelectedTimerIds().size();
    ui->editButton->setVisible(selectedCount == 1);
}

// --- Кнопки знизу ---
void MainWindow::onAddTimer()
{
    // твоя логіка додавання таймера
}

void MainWindow::onStartSelected()
{
    for(int id : getSelectedTimerIds()) manager->startTimer(id);
}

void MainWindow::onStopSelected()
{
    for(int id : getSelectedTimerIds()) manager->pauseTimer(id);
}

void MainWindow::onDeleteSelected()
{
    for(int id : getSelectedTimerIds()) manager->removeTimer(id);
    refreshTimersTable();
}

void MainWindow::onEditSelected()
{
    QSet<int> selected = getSelectedTimerIds();
    if(selected.size() != 1) return;
    int editId = *selected.begin();
    TimerEntry* t = manager->getTimerById(editId);
    if(!t) return;

    EditTimerDialog dlg(this);
    dlg.setTimerData(t);
    if(dlg.exec() == QDialog::Accepted) {
        manager->updateTimer(editId, dlg.getTimerName(), dlg.getDurationSeconds());
        refreshTimersTable();
    }
}

// --- Кнопки всередині таблиці ---
void MainWindow::handleRowStart(int id) { manager->startTimer(id); }
void MainWindow::handleRowStop(int id)  { manager->pauseTimer(id); }
void MainWindow::handleRowDelete(int id) {
    manager->removeTimer(id);
    refreshTimersTable();
}
