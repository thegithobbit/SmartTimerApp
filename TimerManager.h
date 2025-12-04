#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QDateTime>
#include <QUuid>

// Клас, що представляє один таймер/будильник
struct TimerEntry
{
    QString id;
    QString name;
    qint64 durationSeconds; // Загальна тривалість (для таймера) або час спрацювання (для будильника)
    bool isAlarm; // true, якщо це будильник (спрацьовує у вказаний час)
    bool isActive; // true, якщо таймер запущено
    QString actionPath; // Шлях до файлу, який треба запустити при спрацюванні

    // Розраховує час, що залишився (для відображення)
    qint64 remainingSeconds() const {
        if (isAlarm) {
            // Для будильника: durationSeconds - це час спрацювання у секундах з початку епохи
            qint64 now = QDateTime::currentSecsSinceEpoch();
            return durationSeconds - now;
        } else {
            // Для таймера: поточна тривалість, що залишилася
            // Припускаємо, що TimerManager оновлює цю логіку
            return remaining;
        }
    }

    // Приватне поле для внутрішнього використання TimerManager
    qint64 remaining = 0; // Початковий залишок для таймера (оновлюється під час роботи)
};

// Клас, що керує всіма таймерами (КОНТРОЛЕР/МОДЕЛЬ)
class TimerManager : public QObject
{
    Q_OBJECT
public:
    explicit TimerManager(QObject *parent = nullptr);

    // Методи керування
    void addTimer(const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath);
    void deleteTimer(const QString& id);
    void startStopTimer(const QString& id, bool start);
    void startAll();
    void stopAll();
    QList<TimerEntry*> getTimers() const;

signals:
    // Сигнали для MainWindow (View)
    void timerListUpdated(const QList<TimerEntry*>& timers); // Оновлення всієї таблиці
    void timerTicked(const QString& id, qint64 remainingTime); // Оновлення часу однієї комірки
    void timerTimeout(const QString& id, const QString& name, const QString& actionPath); // Спрацювання таймера

private slots:
    void on_timer_tick(); // Основний цикл таймера

private:
    QMap<QString, TimerEntry*> timers; // Зберігає всі таймери за ID
    QTimer *updateTimer; // Таймер для оновлення кожну секунду

    void emitListUpdate();
    void checkAlarmStatus();
};

#endif // TIMERMANAGER_H
