#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include "TimerManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTimer();
    void onStartSelected();
    void onStopSelected();
    void onDeleteSelected();
    void onEditSelected();

    void handleRowStart(int id);
    void handleRowStop(int id);
    void handleRowDelete(int id);

    void refreshTimersTable();
    void updateEditButtonVisibility();

private:
    Ui::MainWindow *ui;
    TimerManager *manager;

    QString formatTime(int totalSeconds) const;
    QSet<int> getSelectedTimerIds() const;
};

#endif // MAINWINDOW_H
