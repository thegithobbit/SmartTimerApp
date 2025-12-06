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

    // ✅ Гетери для доступу до полів
    QLineEdit* getNameEdit() const { return nameEdit; }
    QSpinBox* getHours() const { return durationHours; }
    QSpinBox* getMinutes() const { return durationMinutes; }
    QSpinBox* getSeconds() const { return durationSeconds; }
    void setSaveButtonEnabled(bool enabled) { saveButton->setEnabled(enabled); }

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
