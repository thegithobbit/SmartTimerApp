#include "EditTimerDialog.h"
#include <QGridLayout>
#include <QMessageBox>

EditTimerDialog::EditTimerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Редагувати Таймер"));
    setupUi();

    connect(saveButton, &QPushButton::clicked, this, &EditTimerDialog::on_save_clicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void EditTimerDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGridLayout *inputLayout = new QGridLayout();

    // Назва таймера
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText(tr("Назва таймера"));

    inputLayout->addWidget(new QLabel(tr("Назва:")), 0, 0);
    inputLayout->addWidget(nameEdit, 0, 1, 1, 2);

    // Поля для тривалості
    durationHours = new QSpinBox();
    durationMinutes = new QSpinBox();
    durationSeconds = new QSpinBox();

    durationHours->setRange(0, 99);
    durationMinutes->setRange(0, 59);
    durationSeconds->setRange(0, 59);

    QHBoxLayout *durationLayout = new QHBoxLayout();
    durationLayout->addWidget(new QLabel(tr("Години:")));
    durationLayout->addWidget(durationHours);
    durationLayout->addWidget(new QLabel(tr("Хвилини:")));
    durationLayout->addWidget(durationMinutes);
    durationLayout->addWidget(new QLabel(tr("Секунди:")));
    durationLayout->addWidget(durationSeconds);
    durationLayout->addStretch();

    inputLayout->addWidget(new QLabel(tr("Тривалість:")), 1, 0);
    inputLayout->addLayout(durationLayout, 1, 1, 1, 2);

    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(20);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton = new QPushButton(tr("Зберегти"));
    cancelButton = new QPushButton(tr("Скасувати"));
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

void EditTimerDialog::setTimerData(TimerEntry *entry)
{
    if (!entry) return;

    currentId = QString::number(entry->id);

    nameEdit->setText(entry->name);

    qint64 totalSeconds = entry->durationSeconds;
    durationHours->setValue(totalSeconds / 3600);
    durationMinutes->setValue((totalSeconds % 3600) / 60);
    durationSeconds->setValue(totalSeconds % 60);
}

void EditTimerDialog::on_save_clicked()
{
    QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Назва таймера не може бути порожньою."));
        return;
    }

    qint64 totalDurationSeconds = static_cast<qint64>(durationHours->value()) * 3600 +
                                  static_cast<qint64>(durationMinutes->value()) * 60 +
                                  static_cast<qint64>(durationSeconds->value());

    if (totalDurationSeconds <= 0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Тривалість таймера має бути більшою за нуль."));
        return;
    }

    emit timerEdited(currentId, name, totalDurationSeconds);
    accept();
}
