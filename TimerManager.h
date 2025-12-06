#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QList>

struct TimerEntry {
    int id;
    QString name;
    int durationSeconds;
    int remainingSeconds;
    bool running;
    QTimer* qtimer;
};

class TimerManager : public QObject
{
    Q_OBJECT

public:
    explicit TimerManager(QObject *parent = nullptr);
    ~TimerManager();

    int addTimer(const QString &name, int durationSeconds);
    bool removeTimer(int id);
    bool startTimer(int id);
    bool pauseTimer(int id);
    bool updateTimer(int id, const QString &newName, int newDurationSeconds);

    QVector<TimerEntry> getAllTimers() const;
    // ДОП: повертає вказівники на внутрішні елементи (без копіювання)
    QList<TimerEntry*> getAllTimersPointers();

signals:
    void timerUpdated(int id, int remainingSeconds, bool running);
    void timerFinished(int id);

private slots:
    void handleTick();

private:
    int nextId;
    QVector<TimerEntry> timers;

    TimerEntry* getTimerById(int id);
};

#endif // TIMERMANAGER_H
