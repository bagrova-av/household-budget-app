#include "authorizationwindow.h"
#include "./ui_authorizationwindow.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

AuthorizationWindow::AuthorizationWindow(DatabaseManager* dbManager, QWidget *parent):
    QDialog(parent),
    ui(new Ui::AuthorizationWindow),
    authManager(nullptr)
{
    ui->setupUi(this);
    ui->passwordLine->setEchoMode(QLineEdit::Password); // Скрываем ввод пароля
    ui->statusLabel->setStyleSheet("color: red;");

    authManager = new AuthManager(dbManager);
}

void AuthorizationWindow::on_buttonSignIn_clicked()
{
    QString login = ui->loginLine->text().trimmed();
    QString password = ui->passwordLine->text();

    if (login.isEmpty() || password.isEmpty())
    {
        ui->statusLabel->setText("Логин и пароль обязательны.");
        return;
    }

    if (authManager->authenticate(login, password))
    {
        qDebug() << "Пользователь авторизован";
        accept();
    }
    else
    {
        ui->statusLabel->setText("Неверный логин или пароль.");
        ui->loginLine->clear();
        ui->passwordLine->clear();
        ui->loginLine->setFocus();
    }
}

QString AuthorizationWindow::getUserRole() const
{
    return authManager->getUserRole();
}

AuthorizationWindow::~AuthorizationWindow()
{
    delete ui;
}

