#include "AddTimerDialog.h"
#include <QTime>
#include <QDate>

AddTimerDialog::AddTimerDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Додати новий таймер/будильник"));
    setupUi();
    
    // Встановлюємо початковий стан
    timerMode->setChecked(true); 
    updateSpinBoxesVisibility(); 
    
    // Обробник 5: Підключення сигналів до слотів
    connect(okButton, &QPushButton::clicked, this, &AddTimerDialog::on_okButton_clicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(timerMode, &QRadioButton::toggled, this, &AddTimerDialog::on_timerMode_toggled);
    connect(alarmMode, &QRadioButton::toggled, this, &AddTimerDialog::on_alarmMode_toggled);
}

// Обробник 2: Кнопка OK
void AddTimerDialog::on_okButton_clicked()
{
    // Валідація даних
    if (nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Будь ласка, введіть назву таймера."));
        return;
    }

    qint64 totalSeconds = 0;
    bool isAlarm = alarmMode->isChecked();
    
    if (isAlarm) {
        // Режим будильника: отримуємо UNIX-час спрацювання
        QDateTime dt = alarmTimeEdit->dateTime();
        if (dt <= QDateTime::currentDateTime()) {
            QMessageBox::warning(this, tr("Помилка"), tr("Будильник має бути встановлений на час у майбутньому."));
            return;
        }
        totalSeconds = dt.toSecsSinceEpoch(); // durationSeconds = UNIX-час
    } else {
        // Режим таймера: отримуємо сумарну тривалість в секундах
        totalSeconds = hoursSpinBox->value() * 3600 + 
                       minutesSpinBox->value() * 60 + 
                       secondsSpinBox->value();
        if (totalSeconds <= 0) {
            QMessageBox::warning(this, tr("Помилка"), tr("Тривалість таймера має бути більшою за нуль."));
            return;
        }
    }
    
    // Надсилаємо сигнал з даними до TimerManager через MainWindow
    emit timerAdded(nameEdit->text(), totalSeconds, isAlarm, actionPathEdit->text());
    
    QDialog::accept();
}

// Обробник 3: Перемикання на режим Таймера
void AddTimerDialog::on_timerMode_toggled(bool checked)
{
    if (checked) {
        // Обробник 6: Оновлення видимості
        updateSpinBoxesVisibility();
    }
}

// Обробник 4: Перемикання на режим Будильника
void AddTimerDialog::on_alarmMode_toggled(bool checked)
{
    if (checked) {
        // Обробник 7: Оновлення видимості
        updateSpinBoxesVisibility();
        // Встановлюємо мінімальний час на поточний
        alarmTimeEdit->setDateTime(QDateTime::currentDateTime().addSecs(60));
    }
}

// Обробник 8: Приховання/Показ елементів
void AddTimerDialog::updateSpinBoxesVisibility()
{
    bool isAlarm = alarmMode->isChecked();
    
    hoursSpinBox->setVisible(!isAlarm);
    minutesSpinBox->setVisible(!isAlarm);
    secondsSpinBox->setVisible(!isAlarm);
    timeLabel->setVisible(!isAlarm);

    alarmTimeEdit->setVisible(isAlarm);
}

// Обробник 9: Створення інтерфейсу (11 елементів)
void AddTimerDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 1. Назва
    mainLayout->addWidget(new QLabel(tr("Назва таймера/будильника:")));
    nameEdit = new QLineEdit(); // Елемент 1
    mainLayout->addWidget(nameEdit);

    // Режими
    QHBoxLayout *modeLayout = new QHBoxLayout();
    timerMode = new QRadioButton(tr("Таймер (зворотний відлік)")); // Елемент 2
    alarmMode = new QRadioButton(tr("Будильник (конкретний час)")); // Елемент 3
    modeLayout->addWidget(timerMode);
    modeLayout->addWidget(alarmMode);
    modeLayout->addStretch();
    mainLayout->addLayout(modeLayout);

    // 2. Час таймера (Спін-бокси)
    timeLabel = new QLabel(tr("Тривалість (Год:Хв:Сек):")); // Елемент 4
    mainLayout->addWidget(timeLabel);

    QHBoxLayout *durationLayout = new QHBoxLayout();
    hoursSpinBox = new QSpinBox(); // Елемент 5
    hoursSpinBox->setRange(0, 99);
    minutesSpinBox = new QSpinBox(); // Елемент 6
    minutesSpinBox->setRange(0, 59);
    secondsSpinBox = new QSpinBox(); // Елемент 7
    secondsSpinBox->setRange(0, 59);

    durationLayout->addWidget(hoursSpinBox);
    durationLayout->addWidget(minutesSpinBox);
    durationLayout->addWidget(secondsSpinBox);
    mainLayout->addLayout(durationLayout);

    // 3. Час будильника (DateTimePicker)
    mainLayout->addWidget(new QLabel(tr("Час спрацювання будильника:")));
    alarmTimeEdit = new QDateTimeEdit(); // Елемент 8
    alarmTimeEdit->setCalendarPopup(true);
    alarmTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    alarmTimeEdit->setDateTime(QDateTime::currentDateTime().addSecs(60)); // Час у майбутньому
    mainLayout->addWidget(alarmTimeEdit);
    
    // 4. Шлях дії
    mainLayout->addWidget(new QLabel(tr("Шлях до дії після спрацювання (напр., програма):")));
    actionPathEdit = new QLineEdit(); // Елемент 9
    mainLayout->addWidget(actionPathEdit);

    // Кнопки OK/Cancel
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    okButton = new QPushButton(tr("OK")); // Елемент 10
    cancelButton = new QPushButton(tr("Скасувати")); // Елемент 11
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // Встановлюємо мінімальний розмір
    resize(400, 300);
}