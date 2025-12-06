#include "mainwindow.h"

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
    , numberColumnAdded(false)
{
    // Створюємо UI вручну (варіант А, без ui файлу)
    this->resize(640, 420);

    // Початкова кількість колонок — 5 (без колонки №)
    timerTable = new QTableWidget(this);
    timerTable->setColumnCount(5);
    timerTable->setHorizontalHeaderLabels({"✓", "Назва", "Залишилось", "Статус", "Дії"});
    timerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timerTable->setSelectionMode(QAbstractItemView::NoSelection);
    timerTable->setGeometry(10, 10, 620, 340);
    timerTable->horizontalHeader()->setHighlightSections(false);
    timerTable->horizontalHeader()->setFocusPolicy(Qt::NoFocus);
    timerTable->verticalHeader()->setVisible(false);

    // Кнопки
    addButton = new QPushButton("Додати таймер", this);
    addButton->setGeometry(10, 360, 140, 36);

    startButton = new QPushButton("Старт усіх", this);
    startButton->setGeometry(160, 360, 110, 36);

    stopButton = new QPushButton("Стоп усіх", this);
    stopButton->setGeometry(280, 360, 110, 36);

    deleteButton = new QPushButton("Видалити обране", this);
    deleteButton->setGeometry(400, 360, 120, 36);

    toggleButton = new QPushButton("Старт/Стоп обране", this);
    toggleButton->setGeometry(530, 360, 100, 36);

    // Підключаємо сигнали
    connect(addButton, &QPushButton::clicked, this, &MainWindow::on_addTimer_clicked);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::on_startAll_clicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::on_stopAll_clicked);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedTimers);
    connect(toggleButton, &QPushButton::clicked, this, &MainWindow::toggleSelectedTimers);

    // Сигнали від TimerManager — при оновленні/фініші
    connect(manager, &TimerManager::timerUpdated, this, [this](int, int, bool){
        updateTimerList(manager->getAllTimersPointers());
    });

    connect(manager, &TimerManager::timerFinished, this, [this](int){
        QMessageBox::information(this, "Таймер", "Таймер завершився!");
        updateTimerList(manager->getAllTimersPointers());
    });

    // Початкове заповнення таблиці (порожня)
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

void MainWindow::ensureNumberColumnExists()
{
    if (numberColumnAdded) return;

    // Вставляємо колонку на позицію 0
    timerTable->insertColumn(0);

    // Оновлюємо заголовки: "№", "✓", "Назва", "Залишилось", "Статус", "Дії"
    timerTable->setHorizontalHeaderLabels({"№", "✓", "Назва", "Залишилось", "Статус", "Дії"});

    // Тепер кількість колонок має бути 6
    numberColumnAdded = true;

    // Фіксуємо ширину колонки № (щоб не рвала розмітку)
    timerTable->setColumnWidth(0, 40);
    timerTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
}

// Індекси колонок залежно від того, чи є колонка "№"
int MainWindow::checkboxColumnIndex() const { return numberColumnAdded ? 1 : 0; }
int MainWindow::nameColumnIndex() const { return numberColumnAdded ? 2 : 1; }
int MainWindow::remainingColumnIndex() const { return numberColumnAdded ? 3 : 2; }
int MainWindow::statusColumnIndex() const { return numberColumnAdded ? 4 : 3; }
int MainWindow::actionsColumnIndex() const { return numberColumnAdded ? 5 : 4; }

void MainWindow::on_addTimer_clicked()
{
    QString name = QInputDialog::getText(this, "Новий таймер", "Назва:");
    if (name.isEmpty()) return;

    int secs = QInputDialog::getInt(this, "Час", "Секунди:", 60, 1);
    manager->addTimer(name, secs);

    // Якщо додаємо перший таймер — переконаємось, що з'явиться колонка №
    if (!numberColumnAdded && manager->getAllTimers().size() > 0) {
        ensureNumberColumnExists();
    }

    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::on_startAll_clicked()
{
    auto list = manager->getAllTimersPointers();
    for (auto t : list) manager->startTimer(t->id);
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::on_stopAll_clicked()
{
    auto list = manager->getAllTimersPointers();
    for (auto t : list) manager->pauseTimer(t->id);
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::on_deleteTimer_clicked()
{
    // Видалення одного — за поточною selection (якщо взагалі є)
    auto selected = timerTable->selectedItems();
    if (selected.isEmpty()) return;

    int row = timerTable->row(selected.first());
    int nameCol = nameColumnIndex();
    if (!timerTable->item(row, nameCol)) return;

    int id = timerTable->item(row, nameCol)->data(Qt::UserRole).toInt();
    manager->removeTimer(id);
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::on_toggleTimer_clicked()
{
    auto selected = timerTable->selectedItems();
    if (selected.isEmpty()) return;

    int row = timerTable->row(selected.first());
    int nameCol = nameColumnIndex();
    if (!timerTable->item(row, nameCol)) return;

    int id = timerTable->item(row, nameCol)->data(Qt::UserRole).toInt();
    // знайдемо таймер серед менеджера
    auto list = manager->getAllTimersPointers();
    for (auto t : list) {
        if (t->id == id) {
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);
            break;
        }
    }
    updateTimerList(manager->getAllTimersPointers());
}

// === Оновлення таблиці ===
void MainWindow::updateTimerList(const QList<TimerEntry*>& timers)
{
    // 1) Якщо в нас ще не додана колонка номерів, але зараз маємо хоча б один таймер — додаємо її
    if (!numberColumnAdded && !timers.isEmpty()) {
        ensureNumberColumnExists();
    }

    // 2) Зберігаємо стан чекбоксів (ID тих, що були відмічені)
    QSet<int> checkedIds;
    for (int r = 0; r < timerTable->rowCount(); ++r) {
        QWidget *w = timerTable->cellWidget(r, checkboxColumnIndex());
        if (!w) continue;
        QCheckBox *ch = w->findChild<QCheckBox*>();
        if (ch && ch->isChecked()) {
            // ім'я знаходиться у колонці nameColumnIndex()
            QTableWidgetItem *nameItem = timerTable->item(r, nameColumnIndex());
            if (nameItem) {
                checkedIds.insert(nameItem->data(Qt::UserRole).toInt());
            }
        }
    }

    // 3) Оновлюємо кількість рядків
    timerTable->setRowCount(timers.size());

    // 4) Заповнюємо рядки заново
    for (int i = 0; i < timers.size(); ++i) {
        TimerEntry* t = timers[i];

        // -- checkbox
        QCheckBox *check = new QCheckBox();
        QWidget *checkWidget = new QWidget();
        QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
        checkLayout->addWidget(check);
        checkLayout->setAlignment(Qt::AlignCenter);
        checkLayout->setContentsMargins(0,0,0,0);
        checkWidget->setLayout(checkLayout);
        timerTable->setCellWidget(i, checkboxColumnIndex(), checkWidget);

        // -- name (зберігаємо id у UserRole)
        QTableWidgetItem *nameItem = new QTableWidgetItem(t->name);
        nameItem->setData(Qt::UserRole, t->id);
        // вимикаємо редагування/селекцію для цих items
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        timerTable->setItem(i, nameColumnIndex(), nameItem);

        // -- remaining
        QTableWidgetItem *remItem = new QTableWidgetItem(QString::number(t->remainingSeconds));
        remItem->setFlags(remItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        timerTable->setItem(i, remainingColumnIndex(), remItem);

        // -- status
        QTableWidgetItem *statusItem = new QTableWidgetItem(t->running ? "Біжить" : "Пауза");
        statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        timerTable->setItem(i, statusColumnIndex(), statusItem);

        // -- actions (кнопки)
        QPushButton *toggleBtn = new QPushButton("Старт/Стоп");
        QPushButton *deleteBtn = new QPushButton("Видалити");

        QWidget *actions = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actions);
        layout->addWidget(toggleBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(2, 2, 2, 2);
        actions->setLayout(layout);

        timerTable->setCellWidget(i, actionsColumnIndex(), actions);

        // Логіка кнопок в рядку
        connect(toggleBtn, &QPushButton::clicked, this, [this, t]() {
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);
            updateTimerList(manager->getAllTimersPointers());
        });

        connect(deleteBtn, &QPushButton::clicked, this, [this, t]() {
            manager->removeTimer(t->id);
            updateTimerList(manager->getAllTimersPointers());
        });
    }

    // 5) Відновлюємо стан галочок
    for (int r = 0; r < timerTable->rowCount(); ++r) {
        QTableWidgetItem *nameItem = timerTable->item(r, nameColumnIndex());
        if (!nameItem) continue;
        int id = nameItem->data(Qt::UserRole).toInt();
        QWidget *w = timerTable->cellWidget(r, checkboxColumnIndex());
        if (!w) continue;
        QCheckBox *ch = w->findChild<QCheckBox*>();
        if (!ch) continue;
        ch->setChecked(checkedIds.contains(id));
    }
}

void MainWindow::startSelectedTimers()
{
    for (int r = 0; r < timerTable->rowCount(); ++r) {
        QWidget *w = timerTable->cellWidget(r, checkboxColumnIndex());
        if (!w) continue;
        QCheckBox *ch = w->findChild<QCheckBox*>();
        if (!ch || !ch->isChecked()) continue;
        int id = timerTable->item(r, nameColumnIndex())->data(Qt::UserRole).toInt();
        manager->startTimer(id);
    }
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::stopSelectedTimers()
{
    for (int r = 0; r < timerTable->rowCount(); ++r) {
        QWidget *w = timerTable->cellWidget(r, checkboxColumnIndex());
        if (!w) continue;
        QCheckBox *ch = w->findChild<QCheckBox*>();
        if (!ch || !ch->isChecked()) continue;
        int id = timerTable->item(r, nameColumnIndex())->data(Qt::UserRole).toInt();
        manager->pauseTimer(id);
    }
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::deleteSelectedTimers()
{
    // Видаляємо в зворотному порядку (щоб індекси не змістилися)
    for (int r = timerTable->rowCount() - 1; r >= 0; --r) {
        QWidget *w = timerTable->cellWidget(r, checkboxColumnIndex());
        if (!w) continue;
        QCheckBox *ch = w->findChild<QCheckBox*>();
        if (ch && ch->isChecked()) {
            int id = timerTable->item(r, nameColumnIndex())->data(Qt::UserRole).toInt();
            manager->removeTimer(id);
        }
    }
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::toggleSelectedTimers()
{
    for (int r = 0; r < timerTable->rowCount(); ++r) {
        QWidget *w = timerTable->cellWidget(r, checkboxColumnIndex());
        if (!w) continue;
        QCheckBox *ch = w->findChild<QCheckBox*>();
        if (!ch || !ch->isChecked()) continue;
        int id = timerTable->item(r, nameColumnIndex())->data(Qt::UserRole).toInt();
        // знайдемо та переключимо
        auto list = manager->getAllTimersPointers();
        for (auto t : list) {
            if (t->id == id) {
                if (t->running) manager->pauseTimer(t->id);
                else manager->startTimer(t->id);
                break;
            }
        }
    }
    updateTimerList(manager->getAllTimersPointers());
}
