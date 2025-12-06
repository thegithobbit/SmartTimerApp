#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
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
    void on_addTimerButton_clicked();
    void on_startSelectedButton_clicked();
    void on_pauseSelectedButton_clicked();
    void on_deleteSelectedButton_clicked();
    void on_editSelectedButton_clicked();

    void handleTimerUpdated(int id, int remaining, bool running);
    void handleTimerFinished(int id);

private:
    Ui::MainWindow *ui;          // <- оце ЗНИКЛО у тебе!
    TimerManager *manager;

    void refreshTable();
    QList<int> getSelectedTimerIds() const;
};

#endif // MAINWINDOW_H
