#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>
#include <QSqlQuery>

#include "databasemanager.h"

struct User
{
    int id;
    QString userName;
    QString passwordHash;
};

class AuthManager
{
private:
    QString hashData(const QString& password);

    DatabaseManager* dbManager;
    QSqlQuery *query;

    QString userRole;
    bool isAuthenticated;

public:
    AuthManager(DatabaseManager* dbManager);
    bool authenticate(const QString& login, const QString& password);

    QString getUserRole() const;
    bool getIsAuthenticated() const;
};

#endif // AUTHMANAGER_H
