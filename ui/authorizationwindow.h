#ifndef AUTHORIZATIONWINDOW_H
#define AUTHORIZATIONWINDOW_H

#include <QDialog>
#include <QString>

#include "../app/authmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class AuthorizationWindow;
}
QT_END_NAMESPACE

class AuthorizationWindow : public QDialog
{
    Q_OBJECT

private slots:
    void on_buttonSignIn_clicked();

private:
    Ui::AuthorizationWindow *ui;
    AuthManager *authManager;

public:
    explicit AuthorizationWindow(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~AuthorizationWindow();

    QString getUserRole() const;
};

#endif // AUTHORIZATIONWINDOW_H
