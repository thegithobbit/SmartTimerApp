#include "TimerManager.h"
#include <QDebug>
#include <QCoreApplication> // Для qApp
#include <QFile>
#include <QMessageBox> // Потрібен для попереджень

// --- Конструктор ---

TimerManager::TimerManager(QObject *parent)
    : QObject(parent)
{
    // Ініціалізація внутрішнього таймера, який оновлюватиме всі таймери кожну секунду
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &TimerManager::on_timer_tick);
    updateTimer->start(1000); // 1000 мс = 1 секунда
}

// --- Приватні методи ---

// Основний цикл: оновлення всіх таймерів
void TimerManager::on_timer_tick()
{
    bool listChanged = false;

    // 1. Перевіряємо статус будильників (можливо, настав їхній час стартувати)
    checkAlarmStatus();

    // 2. Ітеруємо всі таймери для оновлення часу
    QList<QString> expiredTimers;

    for (TimerEntry *entry : timers.values()) {
        if (!entry->isActive) {
            continue; // Пропускаємо зупинені
        }

        if (entry->isAlarm) {
            // Для будильника: час, що залишився, розраховується динамічно
            qint64 remaining = entry->remainingSeconds();
            if (remaining <= 0) {
                expiredTimers.append(entry->id);
            }
            emit timerTicked(entry->id, remaining); // Все одно оновлюємо, щоб показати відлік
        } else {
            // Для таймера: зменшуємо залишок
            entry->remaining--;
            emit timerTicked(entry->id, entry->remaining);

            if (entry->remaining <= 0) {
                expiredTimers.append(entry->id);
            }
        }
    }

    // 3. Обробляємо спрацьовані таймери
    for (const QString& id : expiredTimers) {
        TimerEntry *entry = timers.value(id);
        if (entry) {
            // Вимикаємо таймер
            entry->isActive = false;
            listChanged = true;

            // Надсилаємо сигнал про спрацювання
            emit timerTimeout(entry->id, entry->name, entry->actionPath);

            // Якщо це Таймер (не Будильник), видаляємо його
            if (!entry->isAlarm) {
                deleteTimer(entry->id);
            }
        }
    }

    if (listChanged) {
        emitListUpdate();
    }
}

// Перевіряє, чи настав час для старту "будильників"
void TimerManager::checkAlarmStatus()
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    bool listChanged = false;

    for (TimerEntry *entry : timers.values()) {
        if (entry->isAlarm && !entry->isActive) {
            // Перевіряємо, чи настав час будильника (коли час спрацювання <= поточного часу)
            // Примітка: будильник повинен бути активним, якщо його час ще не настав, але він має бути у статусі "Очікування"
            if (entry->durationSeconds <= now) {
                entry->isActive = true; // Запускаємо будильник
                listChanged = true;
            }
        }
    }

    if (listChanged) {
        emitListUpdate();
    }
}

// Надсилає повний список таймерів для оновлення таблиці
void TimerManager::emitListUpdate()
{
    emit timerListUpdated(timers.values());
}

QList<TimerEntry*> TimerManager::getTimers() const
{
    return timers.values();
}

// --- Публічні методи керування ---

void TimerManager::addTimer(const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath)
{
    TimerEntry *newEntry = new TimerEntry();
    newEntry->id = QUuid::createUuid().toString(); // Унікальний ID
    newEntry->name = name;
    newEntry->durationSeconds = durationSeconds; // Тривалість або час спрацювання
    newEntry->isAlarm = isAlarm;
    newEntry->actionPath = actionPath;
    newEntry->isActive = isAlarm; // Будильник стартує одразу (очікує спрацювання), Таймер - ні

    // Для Таймера: встановлюємо початковий залишок
    if (!isAlarm) {
        newEntry->remaining = durationSeconds;
    }

    timers.insert(newEntry->id, newEntry);
    emitListUpdate(); // Повідомляємо GUI про оновлення
}

void TimerManager::deleteTimer(const QString& id)
{
    if (timers.contains(id)) {
        TimerEntry *entry = timers.take(id);
        delete entry;
        emitListUpdate(); // Повідомляємо GUI про оновлення
    }
}

void TimerManager::startStopTimer(const QString& id, bool start)
{
    if (timers.contains(id)) {
        TimerEntry *entry = timers.value(id);
        if (entry->isActive != start) { // Якщо статус змінився
            entry->isActive = start;

            // Якщо ми запускаємо зупинений Таймер, який вже сплив,
            // повертаємо йому повний час (щоб перезапустити).
            if (!entry->isAlarm && start && entry->remaining <= 0) {
                entry->remaining = entry->durationSeconds;
            }

            emitListUpdate(); // Повідомляємо GUI про оновлення
        }
    }
}

void TimerManager::startAll()
{
    bool changed = false;
    for (TimerEntry *entry : timers.values()) {
        if (!entry->isActive) {
            entry->isActive = true;
            changed = true;

            // Логіка для перезапуску вже спливлих таймерів
            if (!entry->isAlarm && entry->remaining <= 0) {
                entry->remaining = entry->durationSeconds;
            }
        }
    }
    if (changed) {
        emitListUpdate();
    }
}

void TimerManager::stopAll()
{
    bool changed = false;
    for (TimerEntry *entry : timers.values()) {
        if (entry->isActive) {
            entry->isActive = false;
            changed = true;
        }
    }
    if (changed) {
        emitListUpdate();
    }
}
