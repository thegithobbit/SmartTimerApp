#include "TimerManager.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>     // Для роботи з JSON документами
#include <QJsonArray>        // Для масивів JSON
#include <QJsonObject>       // Для об'єктів JSON
#include <QStandardPaths>    // Для визначення шляху збереження
#include <QMessageBox>
#include <QProcess>          // Для запуску програм
#include <QDateTime>         // Для роботи з часом (будильники)
#include <QUuid>             // Для генерації унікальних ID
#include <QTimer>            // Для internal QTimer
#include <QtAlgorithms>      // Для qDeleteAll
#include <QDir>

// --- Приватні методи серіалізації (JSON) ---

// Перетворення об'єкта TimerEntry на QJsonObject
QJsonObject TimerManager::timerEntryToJson(const TimerEntry* entry) const
{
    QJsonObject json;
    json["id"] = entry->id;
    json["name"] = entry->name;
    // Використовуємо qint64 для durationSeconds і remaining, щоб зберегти точність
    json["durationSeconds"] = (qint64)entry->durationSeconds;
    json["isAlarm"] = entry->isAlarm;
    json["actionPath"] = entry->actionPath;
    json["isActive"] = entry->isActive;
    json["remaining"] = (qint64)entry->remaining;
    return json;
}

// Перетворення QJsonObject на об'єкт TimerEntry
TimerEntry* TimerManager::timerEntryFromJson(const QJsonObject& json) const
{
    TimerEntry *entry = new TimerEntry();
    entry->id = json["id"].toString();
    entry->name = json["name"].toString();
    // Використовуємо toDouble() для безпечного отримання qint64 з JSON
    entry->durationSeconds = (qint64)json["durationSeconds"].toDouble();
    entry->isAlarm = json["isAlarm"].toBool();
    entry->actionPath = json["actionPath"].toString();
    entry->isActive = json["isActive"].toBool();
    entry->remaining = (qint64)json["remaining"].toDouble();

    // Логіка корекції після завантаження: якщо це Таймер і він був активний,
    // але його час сплив (поки програма була закрита), ми його зупиняємо.
    if (!entry->isAlarm && entry->isActive && entry->remaining <= 0)
    {
        entry->isActive = false;
        // remaining залишаємо 0, щоб користувач міг його перезапустити
    }

    return entry;
}

// --- Конструктор ---

TimerManager::TimerManager(QObject *parent)
    : QObject(parent)
{
    // *** ОНОВЛЕННЯ: Завантажуємо збережені таймери при запуску ***
    loadTimers();

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

    for (TimerEntry *entry : timers.values())
    {
        if (!entry->isActive)
            continue; // Пропускаємо зупинені

        if (entry->isAlarm)
        {
            // Для будильника: час, що залишився, розраховується динамічно
            qint64 remaining = entry->remainingSeconds();
            if (remaining <= 0)
                expiredTimers.append(entry->id);

            emit timerTicked(entry->id, remaining); // Все одно оновлюємо, щоб показати відлік
        }
        else
        {
            // Для таймера: зменшуємо залишок
            entry->remaining--;
            emit timerTicked(entry->id, entry->remaining);

            if (entry->remaining <= 0)
                expiredTimers.append(entry->id);
        }
    }

    // 3. Обробляємо спрацьовані таймери
    for (const QString& id : expiredTimers)
    {
        TimerEntry *entry = timers.value(id);
        if (entry)
        {
            // Вимикаємо таймер
            entry->isActive = false;
            listChanged = true;

            // Надсилаємо сигнал про спрацювання
            emit timerTimeout(entry->id, entry->name, entry->actionPath);

            // Якщо це Таймер (не Будильник), видаляємо його
            if (!entry->isAlarm)
                deleteTimer(entry->id);
        }
    }

    if (listChanged)
    {
        emitListUpdate();
        // *** ОНОВЛЕННЯ: Зберігаємо стан після зміни списку (спрацювання, видалення) ***
        saveTimers();
    }
}

// Перевіряє, чи настав час для старту "будильників"
void TimerManager::checkAlarmStatus()
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    bool listChanged = false;

    for (TimerEntry *entry : timers.values())
    {
        if (entry->isAlarm && !entry->isActive)
        {
            // Перевіряємо, чи настав час будильника (коли час спрацювання <= поточного часу)
            if (entry->durationSeconds <= now)
            {
                entry->isActive = true; // Запускаємо будильник
                listChanged = true;
            }
        }
    }

    if (listChanged)
    {
        emitListUpdate();
        saveTimers();
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

// *** НОВИЙ МЕТОД: Отримати один таймер за ID (для редагування) ***
TimerEntry* TimerManager::getTimerEntry(const QString& id) const
{
    return timers.value(id, nullptr);
}

// --- НОВІ МЕТОДИ: Збереження/завантаження (Persistence) ---

void TimerManager::saveTimers() const
{
    // Визначаємо шлях до файлу (зазвичай у папці AppData)
    QString appName = QCoreApplication::applicationName().isEmpty() ? "TimerApp" : QCoreApplication::applicationName();
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + appName;
    QDir dir;
    if (!dir.exists(dirPath))
        dir.mkpath(dirPath);

    QString filePath = dirPath + "/timers.json";
    QFile saveFile(filePath);

    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Помилка: Не вдалося відкрити файл для збереження таймерів:" << filePath;
        return;
    }

    QJsonArray timerArray;
    for (TimerEntry *entry : timers.values())
        timerArray.append(timerEntryToJson(entry));

    QJsonDocument saveDoc(timerArray);
    saveFile.write(saveDoc.toJson(QJsonDocument::Indented));
    qDebug() << "Таймери збережено:" << filePath;
}

void TimerManager::loadTimers()
{
    QString appName = QCoreApplication::applicationName().isEmpty() ? "TimerApp" : QCoreApplication::applicationName();
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + appName;
    QString filePath = dirPath + "/timers.json";
    QFile loadFile(filePath);

    if (!loadFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Файл таймерів не знайдено або він порожній. Створюємо новий список.";
        return; // Це нормально при першому запуску
    }

    QByteArray data = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    if (loadDoc.isNull() || !loadDoc.isArray())
    {
        qWarning("Помилка: Не вдалося розібрати JSON-документ.");
        return;
    }

    QJsonArray timerArray = loadDoc.array();

    // Очищаємо поточний список перед завантаженням
    qDeleteAll(timers);
    timers.clear();

    for (const QJsonValue &value : timerArray)
    {
        QJsonObject json = value.toObject();
        TimerEntry *entry = timerEntryFromJson(json);
        if (entry)
            timers.insert(entry->id, entry);
    }

    qDebug() << "Завантажено таймерів:" << timers.size();
    emitListUpdate(); // Оновлюємо інтерфейс після завантаження
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

    // Будильник стартує одразу (очікує спрацювання), Таймер - ні
    if (isAlarm)
        newEntry->isActive = (durationSeconds > QDateTime::currentSecsSinceEpoch());
    else
    {
        newEntry->isActive = false;
        newEntry->remaining = durationSeconds; // Для Таймера: встановлюємо початковий залишок
    }

    timers.insert(newEntry->id, newEntry);
    emitListUpdate(); // Повідомляємо GUI про оновлення
    saveTimers();     // Зберігаємо після додавання
}

// *** НОВИЙ МЕТОД: Редагування існуючого таймера ***
void TimerManager::editTimer(const QString& id, const QString& name, qint64 durationSeconds, bool isAlarm, const QString& actionPath)
{
    if (timers.contains(id))
    {
        TimerEntry *entry = timers.value(id);

        // 1. Оновлюємо загальні поля
        entry->name = name;
        entry->actionPath = actionPath;

        // 2. Якщо змінився тип (Таймер <-> Будильник) або час, скидаємо стан
        bool typeChanged = entry->isAlarm != isAlarm;
        bool durationChanged = entry->durationSeconds != durationSeconds;

        entry->isAlarm = isAlarm;
        entry->durationSeconds = durationSeconds;

        if (typeChanged || durationChanged)
        {
            if (isAlarm)
            {
                entry->isActive = (durationSeconds > QDateTime::currentSecsSinceEpoch());
                entry->remaining = 0; // Для будильника залишок не використовується
            }
            else
            {
                entry->remaining = durationSeconds;
                entry->isActive = false; // Зупиняємо, щоб користувач сам його запустив
            }
        }

        emitListUpdate();
        saveTimers(); // Зберігаємо після редагування
    }
}

void TimerManager::deleteTimer(const QString& id)
{
    if (timers.contains(id))
    {
        TimerEntry *entry = timers.take(id);
        delete entry;
        emitListUpdate(); // Повідомляємо GUI про оновлення
        saveTimers();     // Зберігаємо після видалення
    }
}

void TimerManager::startStopTimer(const QString& id, bool start)
{
    if (timers.contains(id))
    {
        TimerEntry *entry = timers.value(id);
        if (entry->isActive != start)
        {
            entry->isActive = start;

            // Якщо ми запускаємо зупинений Таймер, який вже сплив,
            // повертаємо йому повний час (щоб перезапустити).
            if (!entry->isAlarm && start && entry->remaining <= 0)
                entry->remaining = entry->durationSeconds;

            emitListUpdate(); // Повідомляємо GUI про оновлення
            saveTimers();     // Зберігаємо після зміни стану
        }
    }
}

void TimerManager::startAll()
{
    bool changed = false;
    for (TimerEntry *entry : timers.values())
    {
        if (!entry->isActive)
        {
            if (entry->isAlarm && entry->durationSeconds > QDateTime::currentSecsSinceEpoch())
            {
                entry->isActive = true;
                changed = true;
            }
            else if (!entry->isAlarm)
            {
                entry->isActive = true;
                changed = true;

                // Логіка для перезапуску вже спливлих таймерів
                if (entry->remaining <= 0)
                    entry->remaining = entry->durationSeconds;
            }
        }
    }

    if (changed)
    {
        emitListUpdate();
        saveTimers();
    }
}

void TimerManager::stopAll()
{
    bool changed = false;
    for (TimerEntry *entry : timers.values())
    {
        if (entry->isActive)
        {
            entry->isActive = false;
            changed = true;
        }
    }

    if (changed)
    {
        emitListUpdate();
        saveTimers();
    }
}
