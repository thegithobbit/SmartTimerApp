#ifndef ADDTIMERDIALOG_H
#define ADDTIMERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTimeEdit>
#include <QLabel>
#include <QMessageBox> // Використовуємо QMessageBox замість alert()

// Це вікно для додавання нового таймера/будильника (Форма 2/4)
class AddTimerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTimerDialog(QWidget *parent = nullptr);
    // Структура для передачі даних
    struct TimerData {
        QString name;
        qint64 durationSeconds;
        bool isAlarm;
        QString actionPath;
    };

    TimerData getTimerData() const;

signals:
    // Сигнал, що сповіщає головне вікно про необхідність додати таймер
    void timerAdded(const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath); // Обробник 1

private slots:
    // Обробники подій для інтерфейсу
    void on_okButton_clicked(); // Обробник 2: Кнопка OK
    void on_timerMode_toggled(bool checked); // Обробник 3: Перемикання режиму "Таймер"
    void on_alarmMode_toggled(bool checked); // Обробник 4: Перемикання режиму "Будильник"

private:
    // Елементи інтерфейсу (всього 11 елементів)
    QLineEdit *nameEdit;
    QRadioButton *timerMode;
    QRadioButton *alarmMode;
    QSpinBox *hoursSpinBox;
    QSpinBox *minutesSpinBox;
    QSpinBox *secondsSpinBox;
    QDateTimeEdit *alarmTimeEdit;
    QLineEdit *actionPathEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *timeLabel; // Додатковий елемент

    // Приватні методи
    void setupUi();
    void updateSpinBoxesVisibility();
};

#endif // ADDTIMERDIALOG_H