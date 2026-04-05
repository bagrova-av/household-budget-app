#ifndef BALANCEMODEL_H
#define BALANCEMODEL_H

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QDateTime>
#include <QVariant>
#include <QString>

class BalanceModel : public QSqlTableModel
{
    Q_OBJECT
private:
    QSqlQuery *query;

public:
    explicit BalanceModel(QObject* parent = nullptr, QSqlDatabase db = QSqlDatabase());
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool createBalance(const QDateTime& createDate, const QDateTime& periodStart, const QDateTime& periodEnd);
    bool disbandBalance(int balanceId);

    QString getQueryError() const;
};

#endif // BALANCEMODEL_H
