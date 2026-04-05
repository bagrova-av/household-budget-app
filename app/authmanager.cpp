#include "authmanager.h"

#include <QCryptographicHash>
#include <QSqlError>
#include <QDebug>

AuthManager::AuthManager(DatabaseManager* dbManager):
    dbManager(dbManager),
    isAuthenticated(false)
{}

bool AuthManager::authenticate(const QString& login, const QString& password)
{
    QString hashedLogin = hashData(login);
    QString hashedPassword = hashData(password);

    query = new QSqlQuery(dbManager->getDatabase());
    query->prepare("SELECT id, user_role FROM users WHERE login = :login AND password_hash = :password_hash");
    query->bindValue(":login", hashedLogin);
    query->bindValue(":password_hash", hashedPassword);

    if (!query->exec())
    {
        qDebug() << "Ошибка при аутентификации" << query->lastError().text();
        return false;
    }
    if (!query->next())
    {
        qDebug() << "Пользователем с именем " << login << " не найден";
        isAuthenticated = false;
        return false;
    }

    userRole = query->value("user_role").toString();
    qDebug() << "Пользователем с именем " << login << " найден. Его роль: " << userRole;
    isAuthenticated = true;
    return true;
}

QString AuthManager::hashData(const QString& password)
{
    QByteArray passwordData = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

QString AuthManager::getUserRole() const
{
    return userRole;
}
bool AuthManager::getIsAuthenticated() const
{
    return isAuthenticated;
}
