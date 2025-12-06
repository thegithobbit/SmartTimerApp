#ifndef ADDTIMERDIALOG_H
#define ADDTIMERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

class AddTimerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddTimerDialog(QWidget *parent = nullptr);

    QString getName() const;
    int getDurationSeconds() const;

signals:
    void timerCreated(const QString &name, int durationSeconds);

private slots:
    void onCreateClicked();

private:
    QLineEdit *nameEdit;
    QSpinBox *hoursSpin;
    QSpinBox *minutesSpin;
    QSpinBox *secondsSpin;
    QPushButton *createButton;
    QPushButton *cancelButton;
};

#endif // ADDTIMERDIALOG_H