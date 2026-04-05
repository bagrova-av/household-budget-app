#ifndef OPERATIONDIALOG_H
#define OPERATIONDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>

namespace Ui {
class AddOperationDialog;
}

class AddOperationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddOperationDialog(const QMap<int, QString>& articles, QWidget *parent = nullptr);
    ~AddOperationDialog();

    void setInitialValues(const QString& currentArticleName,
                          double currentDebit,
                          double currentCredit,
                          const QDateTime& currentDate);

    QString getArticleName() const;
    double getDebit() const;
    double getCredit() const;
    QDateTime getOperationDate() const;

private:
    Ui::AddOperationDialog *ui;
    QMap<int, QString> articles;
};

#endif // OPERATIONDIALOG_H
