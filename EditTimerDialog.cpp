#include "EditTimerDialog.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDateTime>

// --- Конструктор / Setup UI ---

EditTimerDialog::EditTimerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Редагувати Таймер/Будильник"));
    setupUi();

    // З'єднання
    connect(typeToggle, &QCheckBox::toggled, this, &EditTimerDialog::on_toggleType_toggled);
    connect(saveButton, &QPushButton::clicked, this, &EditTimerDialog::on_save_clicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void EditTimerDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGridLayout *inputLayout = new QGridLayout();

    // 1. Поля вводу
    typeLabel = new QLabel(tr("Тип:"));
    typeToggle = new QCheckBox(tr("Будильник")); // Не відмічено = Таймер
    nameEdit = new QLineEdit();
    actionPathEdit = new QLineEdit();

    nameEdit->setPlaceholderText(tr("Назва таймера (напр., 'Перерва', 'Зустріч')"));
    actionPathEdit->setPlaceholderText(tr("Шлях до програми, яку потрібно запустити (опц.)"));

    QPushButton *browseButton = new QPushButton(tr("..."));
    browseButton->setMaximumWidth(40);
    connect(browseButton, &QPushButton::clicked, [this](){
        QString path = QFileDialog::getOpenFileName(this, tr("Вибрати програму для запуску"));
        if (!path.isEmpty()) {
            actionPathEdit->setText(path);
        }
    });

    // 2. Створення Stacked Widget для часу
    timeStack = new QStackedWidget();

    // 2.1. Варіант "Таймер" (Тривалість)
    durationWidget = new QWidget();
    QHBoxLayout *durationLayout = new QHBoxLayout(durationWidget);
    durationHours = new QSpinBox();
    durationMinutes = new QSpinBox();
    durationSeconds = new QSpinBox();

    durationHours->setRange(0, 99);
    durationMinutes->setRange(0, 59);
    durationSeconds->setRange(0, 59);

    durationLayout->addWidget(new QLabel(tr("Години:")));
    durationLayout->addWidget(durationHours);
    durationLayout->addWidget(new QLabel(tr("Хвилини:")));
    durationLayout->addWidget(durationMinutes);
    durationLayout->addWidget(new QLabel(tr("Секунди:")));
    durationLayout->addWidget(durationSeconds);
    durationLayout->addStretch();
    durationWidget->setLayout(durationLayout);

    timeStack->addWidget(durationWidget); // Індекс 0

    // 2.2. Варіант "Будильник" (Час спрацювання)
    alarmWidget = new QWidget();
    QHBoxLayout *alarmLayout = new QHBoxLayout(alarmWidget);
    alarmDateTime = new QDateTimeEdit(QDateTime::currentDateTime());
    alarmDateTime->setCalendarPopup(true);
    alarmDateTime->setMinimumDateTime(QDateTime::currentDateTime());

    alarmLayout->addWidget(new QLabel(tr("Час спрацювання:")));
    alarmLayout->addWidget(alarmDateTime);
    alarmLayout->addStretch();
    alarmWidget->setLayout(alarmLayout);

    timeStack->addWidget(alarmWidget); // Індекс 1

    // 3. Компонування
    inputLayout->addWidget(typeLabel, 0, 0);
    inputLayout->addWidget(typeToggle, 0, 1, 1, 2);
    inputLayout->addWidget(new QLabel(tr("Назва:")), 1, 0);
    inputLayout->addWidget(nameEdit, 1, 1, 1, 2);
    inputLayout->addWidget(new QLabel(tr("Дія:")), 2, 0);
    inputLayout->addWidget(actionPathEdit, 2, 1);
    inputLayout->addWidget(browseButton, 2, 2);
    inputLayout->addWidget(new QLabel(tr("Час:")), 3, 0);
    inputLayout->addWidget(timeStack, 3, 1, 1, 2);

    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(20);

    // 4. Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton = new QPushButton(tr("Зберегти Зміни"));
    cancelButton = new QPushButton(tr("Скасувати"));

    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

// --- Слоти / Методи керування ---

// Метод для ініціалізації діалогу даними таймера
void EditTimerDialog::setTimerData(TimerEntry *entry)
{
    if (!entry) return;

    currentId = entry->id;
    nameEdit->setText(entry->name);
    actionPathEdit->setText(entry->actionPath);

    typeToggle->setChecked(entry->isAlarm); // Оновлює typeStack через on_toggleType_toggled

    if (entry->isAlarm) {
        // Завантажуємо час спрацювання
        alarmDateTime->setDateTime(QDateTime::fromSecsSinceEpoch(entry->durationSeconds));
    } else {
        // Завантажуємо тривалість
        qint64 totalSeconds = entry->durationSeconds;
        durationHours->setValue(totalSeconds / 3600);
        durationMinutes->setValue((totalSeconds % 3600) / 60);
        durationSeconds->setValue(totalSeconds % 60);
    }
}

// Обробник перемикача типу
void EditTimerDialog::on_toggleType_toggled(bool isAlarm)
{
    if (isAlarm) {
        timeStack->setCurrentIndex(1); // Будильник
    } else {
        timeStack->setCurrentIndex(0); // Таймер
    }
}

// Обробник кнопки "Зберегти"
void EditTimerDialog::on_save_clicked()
{
    QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Помилка вводу"), tr("Назва таймера не може бути порожньою."));
        return;
    }

    bool isAlarm = typeToggle->isChecked();

    // !!! Змінено ім'я локальної змінної, щоб не конфліктувати з QSpinBox* durationSeconds !!!
    qint64 totalDurationSeconds = 0;

    if (isAlarm) {
        // Будильник: durationSeconds - це час спрацювання (Unix timestamp)
        QDateTime targetTime = alarmDateTime->dateTime();
        if (targetTime <= QDateTime::currentDateTime().addSecs(-10)) { // 10 сек запас
            QMessageBox::warning(this, tr("Помилка часу"), tr("Час спрацювання будильника має бути у майбутньому."));
            return;
        }
        totalDurationSeconds = targetTime.toSecsSinceEpoch();

    } else {
        // Таймер: totalDurationSeconds - це загальна тривалість
        int h = durationHours->value();
        int m = durationMinutes->value();
        int s = durationSeconds->value(); // використовуємо QSpinBox* member
        totalDurationSeconds = static_cast<qint64>(h) * 3600 + static_cast<qint64>(m) * 60 + static_cast<qint64>(s);

        if (totalDurationSeconds <= 0) {
            QMessageBox::warning(this, tr("Помилка часу"), tr("Тривалість таймера має бути більшою за нуль."));
            return;
        }
    }

    // Надсилаємо сигнал про редагування
    emit timerEdited(currentId, name, totalDurationSeconds, isAlarm, actionPathEdit->text().trimmed());

    accept(); // Закриваємо діалог
}
