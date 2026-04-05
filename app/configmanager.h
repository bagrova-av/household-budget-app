#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>

struct DbConfig
{
    QString host;
    int port;
    QString dbName;
    QString userName;
    QString password;
};

class ConfigManager
{
private:
    DbConfig dbConfig;
    bool validateConfig() const;

public:
    ConfigManager();

    bool loadConfig(const QString& configFilePath);

    const DbConfig& getDbConfig() const;
    bool getIsValid() const;
    QString getAbsoluteConfigPath(const QString &configPath) const;
};

#endif // CONFIGMANAGER_H
