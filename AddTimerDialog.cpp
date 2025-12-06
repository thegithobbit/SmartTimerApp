#include "AddTimerDialog.h"
#include <QMessageBox>

AddTimerDialog::AddTimerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Новий Таймер");

    nameEdit = new QLineEdit(this);
    hoursSpin = new QSpinBox(this); hoursSpin->setRange(0, 99);
    minutesSpin = new QSpinBox(this); minutesSpin->setRange(0, 59);
    secondsSpin = new QSpinBox(this); secondsSpin->setRange(0, 59);

    createButton = new QPushButton("Створити", this);
    cancelButton = new QPushButton("Скасувати", this);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("Назва:", nameEdit);
    formLayout->addRow("Години:", hoursSpin);
    formLayout->addRow("Хвилини:", minutesSpin);
    formLayout->addRow("Секунди:", secondsSpin);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(createButton, &QPushButton::clicked, this, &AddTimerDialog::onCreateClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString AddTimerDialog::getName() const
{
    return nameEdit->text().trimmed();
}

int AddTimerDialog::getDurationSeconds() const
{
    return hoursSpin->value() * 3600 + minutesSpin->value() * 60 + secondsSpin->value();
}

void AddTimerDialog::onCreateClicked()
{
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Помилка", "Введіть назву таймера!");
        return;
    }
    emit timerCreated(getName(), getDurationSeconds());
    accept();
}