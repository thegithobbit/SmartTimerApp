#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , manager(new TimerManager(this))
{
    // === Створюємо UI вручну ===
    resize(700, 400);

    timerTable = new QTableWidget(this);
    timerTable->setColumnCount(6); // галочка, №, назва, залишилось, статус, дії
    timerTable->setHorizontalHeaderLabels({"✓", "№", "Назва", "Залишилось", "Статус", "Дії"});
    timerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timerTable->setSelectionMode(QAbstractItemView::NoSelection);
    timerTable->horizontalHeader()->setHighlightSections(false);
    timerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    addButton = new QPushButton("Додати таймер", this);
    startButton = new QPushButton("Старт усіх", this);
    stopButton = new QPushButton("Стоп усіх", this);
    deleteButton = new QPushButton("Видалити обране", this);
    toggleButton = new QPushButton("Старт/Стоп обране", this);
    editButton = new QPushButton("Редагувати вибраний", this);
    editButton->setVisible(false); // спочатку невидима

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(timerTable);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(toggleButton);
    buttonLayout->addWidget(editButton);

    mainLayout->addLayout(buttonLayout);

    // === Сигнали ===
    connect(addButton, &QPushButton::clicked, this, &MainWindow::on_addTimer_clicked);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::on_startAll_clicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::on_stopAll_clicked);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedTimers);
    connect(toggleButton, &QPushButton::clicked, this, &MainWindow::toggleSelectedTimers);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::on_editSelected_clicked);

    connect(manager, &TimerManager::timerUpdated, this, [this](int, int, bool){
        updateTimerList(manager->getAllTimersPointers());
        updateEditButtonVisibility();
    });

    connect(manager, &TimerManager::timerFinished, this, [this](int){
        QMessageBox::information(this, "Таймер", "Таймер завершився!");
        updateTimerList(manager->getAllTimersPointers());
        updateEditButtonVisibility();
    });

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
    delete editButton;
}

QList<int> MainWindow::getSelectedRows() const
{
    QList<int> rows;
    for (int i = 0; i < timerTable->rowCount(); ++i)
    {
        QWidget *w = timerTable->cellWidget(i, 0);
        if (!w) continue;
        QCheckBox *check = w->findChild<QCheckBox*>();
        if (check && check->isChecked())
            rows.append(i);
    }
    return rows;
}

void MainWindow::updateEditButtonVisibility()
{
    QList<int> selected = getSelectedRows();
    editButton->setVisible(selected.size() == 1);
}

// === Додати таймер ===
void MainWindow::on_addTimer_clicked()
{
    QString name = QInputDialog::getText(this, "Новий таймер", "Назва:");
    if (name.isEmpty()) return;

    // Перевірка унікальності
    for (auto t : manager->getAllTimersPointers())
        if (t->name == name) {
            QMessageBox::warning(this, "Помилка", "Таймер з такою назвою існує");
            return;
        }

    int secs = QInputDialog::getInt(this, "Час", "Секунди:", 60, 1);
    manager->addTimer(name, secs);
    updateTimerList(manager->getAllTimersPointers());
    updateEditButtonVisibility();
}

// === Старт/Стоп усіх ===
void MainWindow::on_startAll_clicked()
{
    for (auto t : manager->getAllTimersPointers())
        manager->startTimer(t->id);
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::on_stopAll_clicked()
{
    for (auto t : manager->getAllTimersPointers())
        manager->pauseTimer(t->id);
    updateTimerList(manager->getAllTimersPointers());
}

// === Видалити обране ===
void MainWindow::deleteSelectedTimers()
{
    auto rows = getSelectedRows();
    for (int i = rows.size() - 1; i >= 0; --i)
    {
        int id = manager->getAllTimersPointers()[rows[i]]->id;
        manager->removeTimer(id);
    }
    updateTimerList(manager->getAllTimersPointers());
    updateEditButtonVisibility();
}

// === Старт/Стоп обране ===
void MainWindow::toggleSelectedTimers()
{
    auto rows = getSelectedRows();
    for (int row : rows)
    {
        TimerEntry* t = manager->getAllTimersPointers()[row];
        if (t->running) manager->pauseTimer(t->id);
        else manager->startTimer(t->id);
    }
    updateTimerList(manager->getAllTimersPointers());
    updateEditButtonVisibility();
}

// === Старт виділених (для окремих кнопок) ===
void MainWindow::startSelectedTimers()
{
    auto rows = getSelectedRows();
    for (int row : rows)
        manager->startTimer(manager->getAllTimersPointers()[row]->id);
    updateTimerList(manager->getAllTimersPointers());
}

void MainWindow::stopSelectedTimers()
{
    auto rows = getSelectedRows();
    for (int row : rows)
        manager->pauseTimer(manager->getAllTimersPointers()[row]->id);
    updateTimerList(manager->getAllTimersPointers());
}

// === Редагування таймера ===
void MainWindow::on_editSelected_clicked()
{
    auto rows = getSelectedRows();
    if (rows.size() != 1) return;

    int row = rows.first();
    TimerEntry* t = manager->getAllTimersPointers()[row];

    EditTimerDialog dlg(this);
    dlg.setTimerData(t);

    connect(&dlg, &EditTimerDialog::timerEdited, this, [this, t](const QString&, const QString& newName, qint64 newSeconds){
        // Перевірка унікальності
        for (auto other : manager->getAllTimersPointers())
            if (other->name == newName && other != t)
            {
                QMessageBox::warning(this, "Помилка", "Таймер з такою назвою існує");
                return;
            }
        manager->updateTimer(t->id, newName, newSeconds);
        updateTimerList(manager->getAllTimersPointers());
    });

    dlg.exec();
}

// === Оновлення таблиці ===
void MainWindow::updateTimerList(const QList<TimerEntry*>& timers)
{
    timerTable->setRowCount(timers.size());

    for (int i = 0; i < timers.size(); ++i)
    {
        TimerEntry* t = timers[i];

        // === Галочка ===
        QCheckBox *check = new QCheckBox();
        QWidget *checkWidget = new QWidget();
        QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
        checkLayout->addWidget(check);
        checkLayout->setAlignment(Qt::AlignCenter);
        checkLayout->setContentsMargins(0,0,0,0);
        timerTable->setCellWidget(i, 0, checkWidget);

        // === № ===
        timerTable->setItem(i, 1, new QTableWidgetItem(QString::number(i+1)));

        // === Назва ===
        timerTable->setItem(i, 2, new QTableWidgetItem(t->name));

        // === Залишилось ===
        timerTable->setItem(i, 3, new QTableWidgetItem(QString::number(t->remainingSeconds)));

        // === Статус ===
        timerTable->setItem(i, 4, new QTableWidgetItem(t->running ? "Біжить" : "Пауза"));

        // === Кнопки у дії ===
        QPushButton *toggleBtn = new QPushButton("Старт/Стоп");
        QPushButton *deleteBtn = new QPushButton("Видалити");

        QWidget *actions = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actions);
        layout->addWidget(toggleBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(2,2,2,2);
        timerTable->setCellWidget(i, 5, actions);

        connect(toggleBtn, &QPushButton::clicked, this, [this, t](){
            if (t->running) manager->pauseTimer(t->id);
            else manager->startTimer(t->id);
            updateTimerList(manager->getAllTimersPointers());
            updateEditButtonVisibility();
        });

        connect(deleteBtn, &QPushButton::clicked, this, [this, t](){
            manager->removeTimer(t->id);
            updateTimerList(manager->getAllTimersPointers());
            updateEditButtonVisibility();
        });
    }
    updateEditButtonVisibility();
}
