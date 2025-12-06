#include "TimerManager.h"

TimerManager::TimerManager(QObject *parent)
    : QObject(parent), nextId(1)
{
}

TimerManager::~TimerManager()
{
    for (auto &t : timers) {
        delete t.qtimer;
    }
}

TimerEntry* TimerManager::getTimerById(int id)
{
    for (auto &t : timers) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

int TimerManager::addTimer(const QString &name, int durationSeconds)
{
    TimerEntry e;
    e.id = nextId++;
    e.name = name;
    e.durationSeconds = durationSeconds;
    e.remainingSeconds = durationSeconds;
    e.running = false;

    e.qtimer = new QTimer(this);
    e.qtimer->setInterval(1000);
    connect(e.qtimer, &QTimer::timeout, this, &TimerManager::handleTick);

    timers.append(e);
    return e.id;
}

bool TimerManager::removeTimer(int id)
{
    for (int i = 0; i < timers.size(); ++i) {
        if (timers[i].id == id) {
            timers[i].qtimer->stop();
            delete timers[i].qtimer;
            timers.removeAt(i);
            return true;
        }
    }
    return false;
}

bool TimerManager::startTimer(int id)
{
    TimerEntry* t = getTimerById(id);
    if (!t || t->running || t->remainingSeconds <= 0)
        return false;

    t->running = true;
    t->qtimer->start();
    emit timerUpdated(id, t->remainingSeconds, true);
    return true;
}

bool TimerManager::pauseTimer(int id)
{
    TimerEntry* t = getTimerById(id);
    if (!t || !t->running)
        return false;

    t->running = false;
    t->qtimer->stop();
    emit timerUpdated(id, t->remainingSeconds, false);
    return true;
}

bool TimerManager::updateTimer(int id, const QString &newName, int newDurationSeconds)
{
    TimerEntry* t = getTimerById(id);
    if (!t)
        return false;

    bool wasRunning = t->running;
    if (wasRunning) pauseTimer(id);

    t->name = newName;
    t->durationSeconds = newDurationSeconds;
    t->remainingSeconds = newDurationSeconds;

    emit timerUpdated(id, t->remainingSeconds, false);
    return true;
}

QVector<TimerEntry> TimerManager::getAllTimers() const
{
    return timers;
}

// НОВА реалізація: повертає список вказівників на внутрішні елементи
QList<TimerEntry*> TimerManager::getAllTimersPointers()
{
    QList<TimerEntry*> list;
    for (int i = 0; i < timers.size(); ++i) {
        // const_cast тут безпечний, бо ми не змінюємо константність зовні
        list.append(const_cast<TimerEntry*>(&timers[i]));
    }
    return list;
}

void TimerManager::handleTick()
{
    QTimer* senderTimer = qobject_cast<QTimer*>(sender());
    if (!senderTimer) return;

    for (auto &t : timers) {
        if (t.qtimer == senderTimer) {
            t.remainingSeconds -= 1;

            emit timerUpdated(t.id, t.remainingSeconds, t.running);

            if (t.remainingSeconds <= 0) {
                t.qtimer->stop();
                t.running = false;
                emit timerFinished(t.id);
            }
            return;
        }
    }
}
