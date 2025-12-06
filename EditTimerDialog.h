#ifndef EDITTIMERDIALOG_H
#define EDITTIMERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

struct TimerEntry; // forward declaration

class EditTimerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditTimerDialog(QWidget *parent = nullptr);

    void setTimerData(TimerEntry *entry);

    // ✅ Геттери для MainWindow
    QString getTimerName() const;
    qint64 getDurationSeconds() const;
    void setSaveButtonEnabled(bool enabled);
    QLineEdit* getNameEdit() const;

private slots:
    void on_save_clicked();

signals:
    void timerEdited(const QString& id, const QString& name, qint64 durationSeconds);

private:
    QString currentId;

    QLineEdit *nameEdit;
    QSpinBox *durationHours;
    QSpinBox *durationMinutes;
    QSpinBox *durationSeconds;

    QPushButton *saveButton;
    QPushButton *cancelButton;

    void setupUi();
};

#endif // EDITTIMERDIALOG_H
