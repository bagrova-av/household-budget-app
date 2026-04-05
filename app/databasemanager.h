#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>

#include "configmanager.h"

class DatabaseManager
{
private:
    QSqlDatabase database;
    bool isConnected;

public:
    DatabaseManager();

    bool connectToDatabase(const QString& host, int port, const QString dbName, const QString& userName, const QString& password);
    bool connectToDatabase(const ConfigManager& config);
    void disconnectFromDatabase();

    QSqlDatabase getDatabase();

    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    bool getIsConnected() const;

    ~DatabaseManager();
};

#endif // DATABASEMANAGER_H
