#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon> // Для роботи з системним треєм
#include <QCloseEvent>
#include "TimerManager.h"
#include "AddTimerDialog.h"

class TimerManager;
class TimerEntry;
// Клас, що представляє головне вікно та GUI-логіку (View/Controller)
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Обробники подій для кнопок та меню (Обробники 1-5)
    void on_addTimer_clicked(); // Обробник 1: Відкрити вікно додавання
    void on_startAll_clicked(); // Обробник 2: Старт усіх
    void on_stopAll_clicked();  // Обробник 3: Стоп усіх
    void on_deleteTimer_clicked(); // Обробник 4: Видалення обраного
    void on_toggleTimer_clicked(); // Обробник 5: Старт/Стоп обраного
    void on_editTimer_clicked();

    // Слоти для прийому сигналів від TimerManager (Обробники 6-8)
    void updateTimerList(const QList<TimerEntry*>& timers); // Обробник 6: Оновлення таблиці
    void handleTimerTimeout(const QString& id, const QString& name, const QString& actionPath); // Обробник 7: Спрацювання таймера
    void handleTimerAdded(const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath); // Обробник 8: Прийом нового таймера з діалогу
    void handleTimerEdited(const QString& id, const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath);

    // Приватні слоти для таблиці (Обробники 9-10)
    void on_table_cellDoubleClicked(int row, int column); // Обробник 9: Подвійний клік на таймері
    void updateCellTime(const QString& id, qint64 remainingTime); // Обробник 10: Оновлення часу в комірці

private:
    // Приватні компоненти UI
    Ui::MainWindow *ui;
    QTableWidget *timerTable;
    QPushButton *addButton;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *deleteButton;
    QPushButton *toggleButton;
    QSystemTrayIcon *trayIcon; // Елемент 12 (Системний трей)

    // Приватні класи (Наші компоненти)
    TimerManager *manager; // Зв'язок з Моделлю
    AddTimerDialog *addDialog; // Зв'язок з Формою 2/4

    // Приватні методи
    void setupUiCustom();
    QString formatTime(qint64 seconds) const;
    // Обробник 16: Обробка закриття вікна
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
