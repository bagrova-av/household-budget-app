#include "balancemodel.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

BalanceModel::BalanceModel(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{
    setTable("balance");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    select();

    query = new QSqlQuery(db);
}

QVariant BalanceModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlTableModel::data(index, role);

    if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.column() == fieldIndex("create_date"))
    {
        QDateTime dt = value.toDateTime();
        if (dt.isValid())
        {
            return dt.toString("yyyy-MM-dd HH:mm");
        }
    }
    return value;
}

bool BalanceModel::createBalance(const QDateTime& createDate, const QDateTime& periodStart, const QDateTime& periodEnd)
{
    int newBalanceId = 0;
    QDateTime localDate = createDate.toLocalTime();
    QString createDateStr = localDate.toString("yyyy-MM-dd HH:mm");
    query->prepare("CALL create_balance(:balance_create_date, :period_start_date, :period_end_date, :new_balance_id)");
    query->bindValue(":balance_create_date", createDateStr);
    query->bindValue(":period_start_date", periodStart.toLocalTime().toString("yyyy-MM-dd HH:mm"));
    query->bindValue(":period_end_date", periodEnd.toLocalTime().toString("yyyy-MM-dd HH:mm"));
    query->bindValue(":new_balance_id", newBalanceId, QSql::Out);

    if (query->exec())
    {
        // Обновляем модель чтобы показать новый баланс
        select();
        return true;
    }
    else
    {
        qDebug() << "Ошибка создания баланса:" << query->lastError().text();
        return false;
    }
}

bool BalanceModel::disbandBalance(int balanceId)
{
    query->prepare("CALL disband_balance(:balance_disband_id)");
    query->bindValue(":balance_disband_id", balanceId);

    if (query->exec())
    {
        select();
        return true;
    }
    else
    {
        qDebug() << "Ошибка расформирования баланса:" << query->lastError().text();
        return false;
    }
}

QString BalanceModel::getQueryError() const
{
    return query->lastError().databaseText();
}
