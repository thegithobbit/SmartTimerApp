#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QCheckBox>
#include "AddTimerDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), manager(new TimerManager(this))
{
    // Вікно
    resize(700, 450);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Таблиця
    timerTable = new QTableWidget();
    timerTable->setColumnCount(5);
    timerTable->setHorizontalHeaderLabels({"Виділити", "Назва", "Час", "Статус", "Дії"});
    timerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    timerTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(timerTable);

    // Кнопки знизу
    QHBoxLayout *btnLayout = new QHBoxLayout();
    addButton = new QPushButton("Додати");
    startButton = new QPushButton("Старт обрані");
    stopButton = new QPushButton("Стоп обрані");
    deleteButton = new QPushButton("Видалити обрані");
    editButton = new QPushButton("Редагувати");
    editButton->setEnabled(false);

    btnLayout->addWidget(addButton);
    btnLayout->addWidget(startButton);
    btnLayout->addWidget(stopButton);
    btnLayout->addWidget(deleteButton);
    btnLayout->addWidget(editButton);
    mainLayout->addLayout(btnLayout);

    // Підключення кнопок
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddTimer);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartSelected);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopSelected);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteSelected);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditSelected);

    // Сигнали від менеджера
    connect(manager, &TimerManager::timerUpdated, this, &MainWindow::refreshTable);
    connect(manager, &TimerManager::timerFinished, this, &MainWindow::refreshTable);

    refreshTable();
}

MainWindow::~MainWindow()
{
    delete manager;
}

// Формат часу
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

// Оновлення таблиці
void MainWindow::refreshTable()
{
    QList<TimerEntry*> timers = manager->getAllTimersPointers();
    timerTable->setRowCount(timers.size());

    for (int i = 0; i < timers.size(); ++i) {
        TimerEntry* t = timers[i];

        // Чекбокс
        QCheckBox *check = new QCheckBox();
        timerTable->setCellWidget(i, 0, check);
        connect(check, &QCheckBox::stateChanged, this, &MainWindow::updateEditButtonVisibility);

        // Назва
        timerTable->setItem(i, 1, new QTableWidgetItem(t->name));

        // Час
        timerTable->setItem(i, 2, new QTableWidgetItem(formatTime(t->remainingSeconds)));

        // Статус
        timerTable->setItem(i, 3, new QTableWidgetItem(t->running ? "Біжить" : "Пауза"));

        // Дії: Старт/Стоп + Видалити
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(2,2,2,2);

        QPushButton *toggleBtn = new QPushButton("Старт/Стоп");
        QPushButton *deleteBtn = new QPushButton("Видалити");

        actionLayout->addWidget(toggleBtn);
        actionLayout->addWidget(deleteBtn);

        timerTable->setCellWidget(i, 4, actionWidget);

        connect(toggleBtn, &QPushButton::clicked, this, [=]() {
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);
            refreshTable();
        });

        connect(deleteBtn, &QPushButton::clicked, this, [=]() {
            manager->removeTimer(t->id);
            refreshTable();
        });
    }

    updateEditButtonVisibility();
}

// Визначаємо чи можна натиснути "Редагувати"
void MainWindow::updateEditButtonVisibility()
{
    int selectedCount = 0;
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 0));
        if (check && check->isChecked()) selectedCount++;
    }
    editButton->setEnabled(selectedCount == 1);
}

// Дії кнопок
void MainWindow::onAddTimer()
{
    AddTimerDialog dlg(this);

    connect(&dlg, &AddTimerDialog::timerCreated, this, [=](const QString &name, int durationSeconds){
        if (!manager->isNameUnique(name)) {
            QMessageBox::warning(this, "Помилка", "Назва має бути унікальною");
            return;
        }
        manager->addTimer(name, durationSeconds);
        refreshTable();
    });

    dlg.exec();
}

void MainWindow::onStartSelected()
{
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            TimerEntry* t = manager->getAllTimersPointers()[row];
            manager->startTimer(t->id);
        }
    }
    refreshTable();
}

void MainWindow::onStopSelected()
{
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            TimerEntry* t = manager->getAllTimersPointers()[row];
            manager->pauseTimer(t->id);
        }
    }
    refreshTable();
}

void MainWindow::onDeleteSelected()
{
    for (int row = timerTable->rowCount()-1; row >=0 ; --row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            TimerEntry* t = manager->getAllTimersPointers()[row];
            manager->removeTimer(t->id);
        }
    }
    refreshTable();
}

void MainWindow::onEditSelected()
{
    int editId = -1;
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 0));
        if (check && check->isChecked()) {
            editId = manager->getAllTimersPointers()[row]->id;
            break;
        }
    }
    if (editId == -1) return;

    TimerEntry* entry = manager->getTimerById(editId);
    if (!entry) return;

    EditTimerDialog dlg(this);
    dlg.setTimerData(entry);

    connect(&dlg, &EditTimerDialog::timerEdited, this, [=](const QString&, const QString &newName, qint64 duration){
        if (!manager->isNameUnique(newName, editId)) {
            QMessageBox::warning(this, "Помилка", "Назва має бути унікальною");
            return;
        }
        manager->updateTimer(editId, newName, duration);
        refreshTable();
    });

    dlg.exec();
    refreshTable();
}
