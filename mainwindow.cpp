#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EditTimerDialog.h"
#include <QCheckBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSet>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), manager(new TimerManager(this))
{
    ui->setupUi(this);

    // Налаштування таблиці
    ui->timersTable->setColumnCount(4);
    ui->timersTable->setHorizontalHeaderLabels({"Виділити", "№", "Назва", "Час"});
    ui->timersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->timersTable->verticalHeader()->setVisible(false);

    // Кнопки
    ui->editButton->setVisible(false); // спочатку не показуємо

    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddTimer);
    connect(ui->startSelectedButton, &QPushButton::clicked, this, &MainWindow::onStartSelected);
    connect(ui->stopSelectedButton, &QPushButton::clicked, this, &MainWindow::onStopSelected);
    connect(ui->editButton, &QPushButton::clicked, this, &MainWindow::onEditSelected);

    connect(manager, &TimerManager::timerUpdated, this, &MainWindow::refreshTimersTable);
    connect(manager, &TimerManager::timerFinished, this, &MainWindow::refreshTimersTable);

    refreshTimersTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshTimersTable()
{
    QSet<int> selectedIds;
    for (int row = 0; row < ui->timersTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(ui->timersTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            int id = ui->timersTable->item(row, 1)->data(Qt::UserRole).toInt();
            selectedIds.insert(id);
        }
    }

    ui->timersTable->setRowCount(0);
    auto timers = manager->getAllTimersPointers();
    for (int i = 0; i < timers.size(); ++i) {
        ui->timersTable->insertRow(i);

        QCheckBox *checkBox = new QCheckBox();
        ui->timersTable->setCellWidget(i, 0, checkBox);
        if (selectedIds.contains(timers[i]->id)) checkBox->setChecked(true);
        connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::updateEditButtonVisibility);

        QTableWidgetItem *numItem = new QTableWidgetItem(QString::number(i + 1));
        numItem->setData(Qt::UserRole, timers[i]->id);
        ui->timersTable->setItem(i, 1, numItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(timers[i]->name);
        ui->timersTable->setItem(i, 2, nameItem);

        QTableWidgetItem *timeItem = new QTableWidgetItem(formatTime(timers[i]->remainingSeconds));
        ui->timersTable->setItem(i, 3, timeItem);
    }

    updateEditButtonVisibility();
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

void MainWindow::updateEditButtonVisibility()
{
    int selectedCount = 0;
    for (int row = 0; row < ui->timersTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(ui->timersTable->cellWidget(row, 0));
        if (check && check->isChecked()) selectedCount++;
    }
    ui->editButton->setVisible(selectedCount == 1);
}

void MainWindow::onAddTimer()
{
    // Твоя логіка додавання таймера
}

void MainWindow::onStartSelected()
{
    for (int row = 0; row < ui->timersTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(ui->timersTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            int id = ui->timersTable->item(row, 1)->data(Qt::UserRole).toInt();
            manager->startTimer(id);
        }
    }
}

void MainWindow::onStopSelected()
{
    for (int row = 0; row < ui->timersTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(ui->timersTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            int id = ui->timersTable->item(row, 1)->data(Qt::UserRole).toInt();
            manager->pauseTimer(id);
        }
    }
}

void MainWindow::onEditSelected()
{
    int editId = -1;
    for (int row = 0; row < ui->timersTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(ui->timersTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            editId = ui->timersTable->item(row, 1)->data(Qt::UserRole).toInt();
            break;
        }
    }
    if (editId == -1) return;

    TimerEntry* entry = manager->getTimerById(editId);
    if (!entry) return;

    EditTimerDialog dlg(this);
    dlg.setTimerData(entry);

    // Жива перевірка унікальності під час редагування
    connect(dlg.getNameEdit(), &QLineEdit::textChanged, this, [this, &dlg, editId]() {
        QString newName = dlg.getTimerName().trimmed();
        bool unique = manager->isNameUnique(newName, editId);
        dlg.setSaveButtonEnabled(unique);
    });

    if (dlg.exec() == QDialog::Accepted) {
        QString newName = dlg.getTimerName().trimmed();
        qint64 newDuration = dlg.getDurationSeconds();
        manager->updateTimer(editId, newName, newDuration);
        refreshTimersTable();
    }
}
