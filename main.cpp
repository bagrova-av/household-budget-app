#include "ui/authorizationwindow.h"
#include "app/configmanager.h"
#include "app/databasemanager.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString configPath = QDir::currentPath() + "/db_config.ini";
    qDebug() << "Загрузка файла концигурации из:" << configPath;

    ConfigManager config;
    if (!config.loadConfig("db_config.ini"))
    {
        qCritical() << "Ошибка: не удалось загрузить конфигурацию БД";
        return -1;
    }

    DatabaseManager dbManager;
    if (!dbManager.connectToDatabase(config))
    {
        qCritical() << "Ошибка: не удалось подключиться к БД";
        return -1;
    }

    AuthorizationWindow authWindow(&dbManager);
    if (authWindow.exec() == QDialog::Accepted)
    {
        MainWindow w(&dbManager, authWindow.getUserRole());
        w.show();

        return app.exec();
    }
    else
    {
        qDebug() << "Авторизация отменена пользователем";
        return 0;
    }
}
