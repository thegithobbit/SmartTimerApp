#ifndef EDITTIMERDIALOG_H
#define EDITTIMERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "TimerManager.h" // Потрібен TimerEntry

class EditTimerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditTimerDialog(QWidget *parent = nullptr);

    // Ініціалізація діалогу даними таймера
    void setTimerData(TimerEntry *entry);

private slots:
    void on_save_clicked();

signals:
    void timerEdited(const QString& id, const QString& name, qint64 durationSeconds);

private:
    QString currentId; // ID таймера, який редагуємо

    // Елементи керування
    QLineEdit *nameEdit;

    QSpinBox *durationHours;
    QSpinBox *durationMinutes;
    QSpinBox *durationSeconds;

    QPushButton *saveButton;
    QPushButton *cancelButton;

    void setupUi();
};

#endif // EDITTIMERDIALOG_H
