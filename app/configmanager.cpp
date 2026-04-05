#include "configmanager.h"

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QApplication>

ConfigManager::ConfigManager(){}

bool ConfigManager::loadConfig(const QString& configFilePath)
{
    QString absolutePath = getAbsoluteConfigPath(configFilePath);
    if (!QFileInfo::exists(absolutePath))
    {
        return false;
    }

    QSettings settings(absolutePath, QSettings::IniFormat);

    settings.beginGroup("database");

    if (!settings.contains("host") || !settings.contains("port") || !settings.contains("dbname") ||
        !settings.contains("user") || !settings.contains("password"))
    {
        qWarning() << "Неправильные параметры в файле концигурации";
        return false;
    }

    dbConfig.host = settings.value("host").toString();
    dbConfig.port = settings.value("port", 5432).toInt();
    dbConfig.dbName = settings.value("dbname").toString();
    dbConfig.userName = settings.value("user").toString();
    dbConfig.password = settings.value("password").toString();

    if (!validateConfig())
    {
        return false;
    }

    settings.endGroup();
    return true;
}

const DbConfig& ConfigManager::getDbConfig() const
{
    return dbConfig;
}

 bool ConfigManager::validateConfig() const
{
    if (dbConfig.host.isEmpty())
    {
        qWarning() << "Неправильные параметры в файле концигурации: поле хост не может быть пустым";
        return false;
    }
    if (dbConfig.port <= 0 || dbConfig.port >= 65535)
    {
        qWarning() << "Неправильные параметры в файле концигурации: некорретное значение порта";
        return false;
    }
    if (dbConfig.dbName.isEmpty())
    {
        qWarning() << "Неправильные параметры в файле концигурации: поле имени базы данных не может быть пустым";
        return false;
    }
    if (dbConfig.userName.isEmpty())
    {
        qWarning() << "Неправильные параметры в файле концигурации: поле имени пользователя не может быть пустым";
        return false;
    }
    if (dbConfig.password.isEmpty())
    {
        qWarning() << "Неправильные параметры в файле концигурации: поле пароля не может быть пустым";
        return false;
    }

    return true;
}

bool ConfigManager::getIsValid() const
{
    return validateConfig();
}

QString ConfigManager::getAbsoluteConfigPath(const QString &configPath) const
{
    if (QDir::isAbsolutePath(configPath))
    {
        return configPath;
    }

    // Поиск относительно директории приложения
    QString appDir = QApplication::applicationDirPath();

    // Возможные расположения
    QStringList possiblePaths;
    possiblePaths << configPath
                  << appDir + "/" + configPath
                  << appDir + "/../" + configPath
                  << appDir + "/../../" + configPath;

    for (const QString &path : possiblePaths)
    {
        if (QFileInfo::exists(path))
        {
            return path;
        }
    }

    // Если не нашли - возвращаем исходный путь
    return configPath;
}
