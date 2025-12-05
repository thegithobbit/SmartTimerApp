#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>

// ========================
//     КОНСТРУКТОР
// ========================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , manager(new TimerManager(this))
{
    ui->setupUi(this);
    setupUiCustom();

    // Підключення сигналів TimerManager
    connect(manager, &TimerManager::timersUpdated,
            this, &MainWindow::updateTimerList);

    connect(manager, &TimerManager::timerFinished,
            this, &MainWindow::handleTimerTimeout);

    connect(manager, &TimerManager::timerAdded,
            this, &MainWindow::handleTimerAdded);

    connect(manager, &TimerManager::timerEdited,
            this, &MainWindow::handleTimerEdited);
}

MainWindow::~MainWindow()
{
    delete ui;
}


// ========================
//     ІНІЦІАЛІЗАЦІЯ UI
// ========================

void MainWindow::setupUiCustom()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // === таблиця ===
    timerTable = new QTableWidget(this);
    timerTable->setColumnCount(4);
    timerTable->setHorizontalHeaderLabels({"Назва", "Залишилось", "Статус", "Дії"});
    timerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    timerTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(timerTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::on_table_cellDoubleClicked);

    // === кнопки ===
    QHBoxLayout *btnLayout = new QHBoxLayout();

    addButton = new QPushButton("Додати");
    startButton = new QPushButton("Старт усіх");
    stopButton = new QPushButton("Стоп усіх");
    deleteButton = new QPushButton("Видалити");
    toggleButton = new QPushButton("Старт/Стоп");

    btnLayout->addWidget(addButton);
    btnLayout->addWidget(startButton);
    btnLayout->addWidget(stopButton);
    btnLayout->addWidget(deleteButton);
    btnLayout->addWidget(toggleButton);

    // Підключення кнопок (слоти з .h)
    connect(addButton, &QPushButton::clicked,
            this, &MainWindow::on_addTimer_clicked);

    connect(startButton, &QPushButton::clicked,
            this, &MainWindow::on_startAll_clicked);

    connect(stopButton, &QPushButton::clicked,
            this, &MainWindow::on_stopAll_clicked);

    connect(deleteButton, &QPushButton::clicked,
            this, &MainWindow::on_deleteTimer_clicked);

    connect(toggleButton, &QPushButton::clicked,
            this, &MainWindow::on_toggleTimer_clicked);

    // === trey icon (як у твоєму .h) ===
    trayIcon = new QSystemTrayIcon(QIcon(), this);
    trayIcon->setToolTip("Timer App");

    // === складання інтерфейсу ===
    mainLayout->addWidget(timerTable);
    mainLayout->addLayout(btnLayout);

    setCentralWidget(central);
}


// ========================
//   СЛОТИ – ТВОЇ З mainwindow.h
// ========================

void MainWindow::on_addTimer_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Новий таймер", "Назва:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    int secs = QInputDialog::getInt(this, "Час", "Секунди:", 60, 1);
    manager->addTimer(name, secs);
}

void MainWindow::on_startAll_clicked()
{
    manager->startAllTimers();
}

void MainWindow::on_stopAll_clicked()
{
    manager->stopAllTimers();
}

void MainWindow::on_deleteTimer_clicked()
{
    int row = timerTable->currentRow();
    if (row < 0) return;

    QString id = timerTable->item(row, 0)->data(Qt::UserRole).toString();
    manager->removeTimer(id);
}

void MainWindow::on_toggleTimer_clicked()
{
    int row = timerTable->currentRow();
    if (row < 0) return;

    QString id = timerTable->item(row, 0)->data(Qt::UserRole).toString();
    manager->toggleTimer(id);
}

void MainWindow::on_editTimer_clicked()
{
    int row = timerTable->currentRow();
    if (row < 0) return;

    QString id = timerTable->item(row, 0)->data(Qt::UserRole).toString();
    TimerEntry *entry = manager->getTimerById(id);
    if (!entry) return;

    bool ok;
    QString newName = QInputDialog::getText(this, "Редагувати", "Назва:",
                                            QLineEdit::Normal, entry->name, &ok);
    if (!ok || newName.isEmpty()) return;

    int newSecs = QInputDialog::getInt(this, "Час", "Секунди:", entry->durationSeconds, 1);

    manager->editTimer(id, newName, newSecs);
}


// ========================
//  ОНОВЛЕННЯ ТАБЛИЦІ
// ========================

void MainWindow::updateTimerList(const QList<TimerEntry*> &timers)
{
    timerTable->setRowCount(0);

    for (TimerEntry* t : timers)
    {
        int row = timerTable->rowCount();
        timerTable->insertRow(row);

        QTableWidgetItem *nameItem = new QTableWidgetItem(t->name);
        nameItem->setData(Qt::UserRole, t->id);
        timerTable->setItem(row, 0, nameItem);

        timerTable->setItem(row, 1, new QTableWidgetItem(formatTime(t->remainingTime)));
        timerTable->setItem(row, 2, new QTableWidgetItem(t->isRunning ? "Біжить" : "Пауза"));

        QPushButton *toggleBtn = new QPushButton(t->isRunning ? "Пауза" : "Старт");
        connect(toggleBtn, &QPushButton::clicked, this, [=]() {
            manager->toggleTimer(t->id);
        });

        QWidget *actions = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actions);
        layout->addWidget(toggleBtn);
        layout->setContentsMargins(2, 2, 2, 2);
        actions->setLayout(layout);

        timerTable->setCellWidget(row, 3, actions);
    }
}


// ========================
//  ОНОВЛЕННЯ ОДНІЄЇ КОМІРКИ
// ========================

void MainWindow::updateCellTime(const QString &id, qint64 remainingTime)
{
    for (int r = 0; r < timerTable->rowCount(); r++) {
        if (timerTable->item(r, 0)->data(Qt::UserRole).toString() == id) {
            timerTable->item(r, 1)->setText(formatTime(remainingTime));
            return;
        }
    }
}


// ========================
//  СПРАЦЮВАННЯ ТАЙМЕРА
// ========================

void MainWindow::handleTimerTimeout(const QString &id, const QString &name, const QString &actionPath)
{
    QMessageBox::information(this, "Таймер", "Таймер \"" + name + "\" завершився!");
}

void MainWindow::handleTimerAdded(const QString &name, qint64 durationSeconds, bool, const QString&)
{
    Q_UNUSED(name)
    Q_UNUSED(durationSeconds)
}

void MainWindow::handleTimerEdited(const QString&, const QString&, qint64, bool, const QString&)
{
    // Нічого не треба робити тут
}


// ========================
//   ПОДВІЙНИЙ КЛІК
// ========================

void MainWindow::on_table_cellDoubleClicked(int row, int)
{
    if (row < 0) return;

    QString id = timerTable->item(row, 0)->data(Qt::UserRole).toString();
    manager->toggleTimer(id);
}


// ========================
//  ФОРМАТУВАННЯ ЧАСУ
// ========================

QString MainWindow::formatTime(qint64 seconds) const
{
    int m = seconds / 60;
    int s = seconds % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}


// ========================
//     ЗАКРИТТЯ ВІКНА
// ========================

void MainWindow::closeEvent(QCloseEvent *event)
{
    trayIcon->showMessage("Таймери", "Додаток працює у фоні.");
    event->ignore();
}
