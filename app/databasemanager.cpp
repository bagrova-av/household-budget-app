#include "databasemanager.h"

#include <QSqlError>

DatabaseManager::DatabaseManager() :
    isConnected(false)
{}

bool DatabaseManager::connectToDatabase(const QString& host, int port, const QString dbName, const QString& userName, const QString& password)
{
    database = QSqlDatabase::addDatabase("QPSQL");
    database.setHostName(host);
    database.setPort(port);
    database.setDatabaseName(dbName);
    database.setUserName(userName);
    database.setPassword(password);

    if (!database.open())
    {
        qDebug() << database.lastError().text();
        return false;
    }

    qDebug() << "Успешное подключение к БД";
    isConnected = true;
    return true;
}
bool DatabaseManager::connectToDatabase(const ConfigManager& config)
{
    if (!config.getIsValid())
    {
        return false;
    }
    DbConfig dbConfig = config.getDbConfig();
    return connectToDatabase(dbConfig.host, dbConfig.port, dbConfig.dbName, dbConfig.userName, dbConfig.password);
}

void DatabaseManager::disconnectFromDatabase()
{
    if (database.isOpen())
    {
        database.close();
        isConnected = false;
        qDebug() << "Отключение от БД";
    }
}

QSqlDatabase DatabaseManager::getDatabase()
{
    return database;
}

bool DatabaseManager::beginTransaction()
{
    if (!isConnected)
    {
        qDebug() << "Ошибка: нет подключения к БД";
        return false;
    }
    return database.transaction();
}
bool DatabaseManager::commitTransaction()
{
    if (!isConnected)
    {
        qDebug() << "Ошибка: нет подключения к БД";
        return false;
    }
    return database.commit();
}
bool DatabaseManager::rollbackTransaction()
{
    if (!isConnected)
    {
        qDebug() << "Ошибка: нет подключения к БД";
        return false;
    }
    return database.rollback();
}

bool DatabaseManager::getIsConnected() const
{
    return database.isOpen();
}


DatabaseManager::~DatabaseManager()
{
    disconnectFromDatabase();
}
