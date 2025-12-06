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
    resize(750, 450);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Таблиця
    timerTable = new QTableWidget();
    timerTable->setColumnCount(6);
    timerTable->setHorizontalHeaderLabels({"№", "Виділити", "Назва", "Час", "Статус", "Дії"});
    timerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    timerTable->verticalHeader()->setVisible(false);
    timerTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // вимикаємо редагування
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

void MainWindow::refreshTable()
{
    // Зберігаємо стан виділення
    QMap<int, bool> selectedMap;
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 1));
        if (check) selectedMap[row] = check->isChecked();
    }

    QList<TimerEntry*> timers = manager->getAllTimersPointers();
    timerTable->setRowCount(timers.size());

    for (int i = 0; i < timers.size(); ++i) {
        TimerEntry* t = timers[i];

        // № рядка
        timerTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));

        // Чекбокс
        QCheckBox *check = new QCheckBox();
        check->setProperty("timerId", t->id);

        // Відновлюємо стан чекбокса
        if (selectedMap.contains(i)) check->setChecked(selectedMap[i]);

        timerTable->setCellWidget(i, 1, check);
        connect(check, &QCheckBox::stateChanged, this, &MainWindow::updateEditButtonVisibility);

        // Назва
        timerTable->setItem(i, 2, new QTableWidgetItem(t->name));

        // Час
        timerTable->setItem(i, 3, new QTableWidgetItem(formatTime(t->remainingSeconds)));

        // Статус
        timerTable->setItem(i, 4, new QTableWidgetItem(t->running ? "Біжить" : "Пауза"));

        // Дії: Старт/Стоп + Видалити
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(2, 2, 2, 2);

        QPushButton *toggleBtn = new QPushButton("Старт/Стоп");
        QPushButton *deleteBtn = new QPushButton("Видалити");

        actionLayout->addWidget(toggleBtn);
        actionLayout->addWidget(deleteBtn);
        timerTable->setCellWidget(i, 5, actionWidget);

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

void MainWindow::updateEditButtonVisibility()
{
    int selectedCount = 0;
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 1)); // колонка 1 = чекбокс
        if (check && check->isChecked()) selectedCount++;
    }
    editButton->setEnabled(selectedCount == 1);
}

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
    QList<TimerEntry*> timers = manager->getAllTimersPointers();
    QVector<int> selectedIds;

    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 1));
        if (check && check->isChecked()) selectedIds.append(timers[row]->id);
    }

    for (int id : selectedIds) manager->startTimer(id);

    // Скидаємо чекбокси
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 1));
        if (check) check->setChecked(false);
    }

    refreshTable();
}


void MainWindow::onStopSelected()
{
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 1)); // колонка з чекбоксом
        if (check && check->isChecked()) {
            int id = check->property("timerId").toInt();
            manager->pauseTimer(id);

            // Скидаємо виділення лише для тих, що зупинили
            check->setChecked(false);
        }
    }
    refreshTable();
}


void MainWindow::onDeleteSelected()
{
    for (int row = timerTable->rowCount() - 1; row >= 0; --row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 1));
        if (check && check->isChecked()) {
            int id = check->property("timerId").toInt();
            manager->removeTimer(id);
        }
    }
    refreshTable();
}

void MainWindow::onEditSelected()
{
    int editId = -1;
    for (int row = 0; row < timerTable->rowCount(); ++row) {
        QCheckBox *check = qobject_cast<QCheckBox*>(timerTable->cellWidget(row, 1));
        if (check && check->isChecked()) {
            editId = check->property("timerId").toInt();
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
