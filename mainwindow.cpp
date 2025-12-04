#include "mainwindow.h"
#include "ui_mainwindow.h" // Все ще потрібен для ініціалізації Qt
#include <QHeaderView>
#include <QDateTime>
#include <QProcess>
#include <QCloseEvent>

// --- Конструктор / Деструктор ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Ініціалізація компонентів
    manager = new TimerManager(this); // Ініціалізація Моделі
    addDialog = new AddTimerDialog(this); // Ініціалізація форми додавання

    setupUiCustom(); // Програмне створення UI

    // 2. З'єднання GUI -> Manager (Обробники 1-5, 8)
    connect(addButton, &QPushButton::clicked, this, &MainWindow::on_addTimer_clicked);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::on_startAll_clicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::on_stopAll_clicked);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::on_deleteTimer_clicked);
    connect(toggleButton, &QPushButton::clicked, this, &MainWindow::on_toggleTimer_clicked);

    // З'єднання Dialog -> Manager (через MainWindow)
    connect(addDialog, &AddTimerDialog::timerAdded, this, &MainWindow::handleTimerAdded); // Обробник 8

    // 3. З'єднання Manager -> GUI (Обробники 6-7, 10)
    connect(manager, &TimerManager::timerListUpdated, this, &MainWindow::updateTimerList); // Обробник 6
    connect(manager, &TimerManager::timerTimeout, this, &MainWindow::handleTimerTimeout); // Обробник 7
    connect(manager, &TimerManager::timerTicked, this, &MainWindow::updateCellTime); // Обробник 10

    // З'єднання для таблиці
    connect(timerTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::on_table_cellDoubleClicked); // Обробник 9
}

MainWindow::~MainWindow()
{
    delete ui;
    // manager та addDialog мають parent, тому видаляються автоматично, але краще явно:
    // delete manager;
    // delete addDialog;
}

// --- Приватні методи ---

// Обробник 11: Форматування часу
QString MainWindow::formatTime(qint64 seconds) const
{
    if (seconds < 0) return "---";
    qint64 h = seconds / 3600;
    qint64 m = (seconds % 3600) / 60;
    qint64 s = seconds % 60;
    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

// Обробник 12: Програмне створення UI (21 елемент)
void MainWindow::setupUiCustom()
{
    // Головний контейнер
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Панель кнопок (QHBoxLayout)
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // Елементи 1-5 (Кнопки)
    addButton = new QPushButton(tr("➕ Додати Таймер/Будильник")); // Елемент 1
    startButton = new QPushButton(tr("▶️ Запустити ВСІ")); // Елемент 2
    stopButton = new QPushButton(tr("⏸️ Зупинити ВСІ")); // Елемент 3
    deleteButton = new QPushButton(tr("❌ Видалити Обраний")); // Елемент 4
    toggleButton = new QPushButton(tr("⏯️ Старт/Стоп Обраний")); // Елемент 5

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(toggleButton);

    mainLayout->addLayout(buttonLayout);

    // Таблиця таймерів (QTableWidget) - Елемент 6
    timerTable = new QTableWidget();
    timerTable->setColumnCount(6); // 6 колонок (Name, Type, Duration/Target, Remaining, Status, ID)
    timerTable->setHorizontalHeaderLabels({
        tr("Назва"),
        tr("Тип"),
        tr("Тривалість / Час"),
        tr("Залишок"),
        tr("Статус"),
        tr("ID") // Прихована колонка для ідентифікатора
    });

    // Налаштування таблиці (Елементи 7-10)
    timerTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // Елемент 7
    timerTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Елемент 8
    timerTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Елемент 9
    timerTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Елемент 10
    timerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    timerTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Заборона редагування

    // Ховаємо колонку ID
    timerTable->setColumnHidden(5, true);

    mainLayout->addWidget(timerTable);

    setCentralWidget(centralWidget);
    setWindowTitle(tr("Smart Timer App"));
    resize(800, 600);

    // Системний трей (Елемент 12)
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icon/timer_icon.ico")); // TODO: Тут можна додати ресурсний файл, поки що заглушка
    trayIcon->setToolTip(tr("Smart Timer App: Активні таймери"));
    trayIcon->show();

    // Меню трею (Елементи 13-17)
    QMenu *trayMenu = new QMenu(this);
    QAction *showAction = new QAction(tr("Показати вікно"), this); // Елемент 13
    QAction *startAllAction = new QAction(tr("Запустити всі"), this); // Елемент 14
    QAction *stopAllAction = new QAction(tr("Зупинити всі"), this); // Елемент 15
    QAction *quitAction = new QAction(tr("Вихід"), this); // Елемент 16
    trayMenu->addAction(showAction);
    trayMenu->addAction(startAllAction);
    trayMenu->addAction(stopAllAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu); // Елемент 17

    // Обробник 13-15: Керування треєм
    connect(showAction, &QAction::triggered, this, &MainWindow::showNormal);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(startAllAction, &QAction::triggered, manager, &TimerManager::startAll);
    connect(stopAllAction, &QAction::triggered, manager, &TimerManager::stopAll);
}

// --- Слоти (Обробники подій) ---

// Обробник 1: Відкрити вікно додавання
void MainWindow::on_addTimer_clicked()
{
    addDialog->exec();
}

// Обробник 2: Старт усіх
void MainWindow::on_startAll_clicked()
{
    manager->startAll();
}

// Обробник 3: Стоп усіх
void void MainWindow::on_stopAll_clicked()
{
    manager->stopAll();
}

// Обробник 4: Видалення обраного
void MainWindow::on_deleteTimer_clicked()
{
    QModelIndexList selectedRows = timerTable->selectionModel()->selectedRows();
    if (!selectedRows.isEmpty()) {
        int row = selectedRows.first().row();
        QString id = timerTable->item(row, 5)->text(); // ID знаходиться в 5-й прихованій колонці
        manager->deleteTimer(id);
    } else {
        QMessageBox::warning(this, tr("Помилка"), tr("Виберіть таймер для видалення."));
    }
}

// Обробник 5: Старт/Стоп обраного
void MainWindow::on_toggleTimer_clicked()
{
    QModelIndexList selectedRows = timerTable->selectionModel()->selectedRows();
    if (!selectedRows.isEmpty()) {
        int row = selectedRows.first().row();
        QString id = timerTable->item(row, 5)->text();
        QString status = timerTable->item(row, 4)->text();

        bool start = (status == tr("Зупинено")) || (status == tr("Очікування"));
        manager->startStopTimer(id, start);
    } else {
        QMessageBox::warning(this, tr("Помилка"), tr("Виберіть таймер для Старт/Стоп."));
    }
}

// Обробник 6: Оновлення таблиці (Прийом даних від TimerManager)
void MainWindow::updateTimerList(const QList<TimerEntry*>& timers)
{
    timerTable->setRowCount(timers.size());
    for (int i = 0; i < timers.size(); ++i) {
        TimerEntry *entry = timers[i];

        // Колонка 0: Назва
        timerTable->setItem(i, 0, new QTableWidgetItem(entry->name));

        // Колонка 1: Тип
        QString type = entry->isAlarm ? tr("Будильник") : tr("Таймер");
        timerTable->setItem(i, 1, new QTableWidgetItem(type));

        // Колонка 2: Тривалість / Час спрацювання
        QString durationOrTarget;
        if (entry->isAlarm) {
            durationOrTarget = QDateTime::fromSecsSinceEpoch(entry->durationSeconds).toString("yyyy-MM-dd HH:mm:ss");
        } else {
            durationOrTarget = formatTime(entry->durationSeconds);
        }
        timerTable->setItem(i, 2, new QTableWidgetItem(durationOrTarget));

        // Колонка 3: Залишок (початкове значення)
        timerTable->setItem(i, 3, new QTableWidgetItem(formatTime(entry->remainingSeconds())));

        // Колонка 4: Статус
        QString status = entry->isActive ? tr("Активний") : tr("Зупинено");
        if (entry->isAlarm && !entry->isActive) {
            status = tr("Очікування"); // Будильник, який ще не стартував
        }
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        if (entry->isActive) {
            statusItem->setBackground(QBrush(Qt::green));
        } else {
            statusItem->setBackground(QBrush(Qt::yellow));
        }
        timerTable->setItem(i, 4, statusItem);

        // Колонка 5: ID (Прихована)
        timerTable->setItem(i, 5, new QTableWidgetItem(entry->id));
    }
}

// Обробник 7: Спрацювання таймера
void MainWindow::handleTimerTimeout(const QString& id, const QString& name, const QString& actionPath)
{
    QString message = tr("Таймер спрацював: %1. Дія: %2").arg(name, actionPath.isEmpty() ? tr("немає") : actionPath);
    trayIcon->showMessage(tr("УВАГА!"), message, QSystemTrayIcon::Information, 5000); // Повідомлення в трей

    QMessageBox::information(this, tr("Таймер Спрацював"), message);

    if (!actionPath.isEmpty()) {
        QProcess::startDetached(actionPath); // Запуск зовнішньої програми
    }
}

// Обробник 8: Прийом нового таймера з діалогу
void MainWindow::handleTimerAdded(const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath)
{
    manager->addTimer(name, durationSeconds, isAlarm, actionPath);
}

// Обробник 9: Подвійний клік (можна використовувати для редагування)
void MainWindow::on_table_cellDoubleClicked(int row, int column)
{
    // Поки що заглушка, але тут буде виклик EditTimerDialog (Форма 3/4)
    qDebug() << "Подвійний клік на таймері з ID:" << timerTable->item(row, 5)->text();
    // Ми не будемо тут викликати EditTimerDialog, поки його не створимо.
}

// Обробник 10: Оновлення часу в комірці
void MainWindow::updateCellTime(const QString& id, qint64 remainingTime)
{
    // Шукаємо рядок за ID
    for (int i = 0; i < timerTable->rowCount(); ++i) {
        if (timerTable->item(i, 5)->text() == id) {
            // Оновлюємо колонку 3 (Залишок)
            QTableWidgetItem *item = timerTable->item(i, 3);
            if (item) {
                item->setText(formatTime(remainingTime));
            }
            // Оновлюємо підказку в треї
            if (remainingTime > 0) {
                trayIcon->setToolTip(tr("Активний таймер: %1. Залишок: %2")
                                         .arg(timerTable->item(i, 0)->text(), formatTime(remainingTime)));
            }
            break;
        }
    }
}

// Обробник 16: Обробка закриття вікна
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide(); // Приховуємо вікно у трей
        event->ignore(); // Ігноруємо закриття
        trayIcon->showMessage(
            tr("Smart Timer App"),
            tr("Програма продовжує працювати у фоновому режимі."),
            QSystemTrayIcon::Information,
            2000
            );
    } else {
        event->accept(); // Закриваємо повністю
    }
}
