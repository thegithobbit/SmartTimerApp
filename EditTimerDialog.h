#ifndef EDITTIMERDIALOG_H
#define EDITTIMERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QStackedWidget>
#include <QVBoxLayout>
#include "TimerManager.h" // Потрібен TimerEntry

class EditTimerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditTimerDialog(QWidget *parent = nullptr);
    
    // Метод для ініціалізації діалогу даними таймера, що редагується
    void setTimerData(TimerEntry *entry);
    
private slots:
    void on_toggleType_toggled(bool checked);
    void on_save_clicked();

signals:
    // Сигнал, що надсилається при успішному редагуванні
    void timerEdited(const QString& id, const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath);

private:
    QString currentId; // ID таймера, який ми редагуємо

    // Елементи керування
    QLabel *typeLabel; 
    QCheckBox *typeToggle; // false = Таймер (тривалість), true = Будильник (час)
    QLineEdit *nameEdit;
    QLineEdit *actionPathEdit;
    
    // Stacked Widget для перемикання між полями вводу часу
    QStackedWidget *timeStack;
    
    // Для Таймера (Тривалість)
    QWidget *durationWidget;
    QSpinBox *durationHours;
    QSpinBox *durationMinutes;
    QSpinBox *durationSeconds;
    
    // Для Будильника (Час спрацювання)
    QWidget *alarmWidget;
    QDateTimeEdit *alarmDateTime;

    QPushButton *saveButton;
    QPushButton *cancelButton;

    void setupUi();
};

#endif // EDITTIMERDIALOG_H