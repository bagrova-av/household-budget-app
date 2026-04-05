#include "operationmodel.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>

OperationModel::OperationModel(QObject* parent, QSqlDatabase db):
    QSqlRelationalTableModel(parent, db)
{
    setTable("operations");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setRelation(fieldIndex("article_id"), QSqlRelation("articles", "id", "name"));
    select();

    query = new QSqlQuery(db);
}

QVariant OperationModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlRelationalTableModel::data(index, role);

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        int dateColumn = fieldIndex("create_date");
        if (index.column() == dateColumn)
        {
            QDateTime dt = value.toDateTime();

            if (dt.isValid())
            {
                return dt.toString("yyyy-MM-dd HH:mm");
            }
        }
    }

    return value;
}

bool OperationModel::addOperation(const QString& articleName, double debit, double credit, const QDateTime& date)
{
    int articleId = getArticleIdByName(articleName);
    qDebug() << "Статья с именем" << articleName << "найдена";
    qDebug() << "Ошибка SQL при добавлении операции:" << query->lastError().text();
    if (articleId == -1)
    {
        qDebug() << "Статья с именем" << articleName << "не найдена";
        return false;
    }

    QDateTime localDate = date.toLocalTime();
    QString dateStr = localDate.toString("yyyy-MM-dd HH:mm");
    query->prepare("INSERT INTO operations (article_id, debit, credit, create_date, balance_id) "
                  "VALUES (:id, :debit, :credit, :create_date, NULL)");
    query->bindValue(":id", articleId);
    query->bindValue(":debit", debit);
    query->bindValue(":credit", credit);
    query->bindValue(":create_date", dateStr);

    if (!query->exec())
    {
        qDebug() << "Ошибка SQL при добавлении операции:" << query->lastError().text();
        return false;
    }

    if (!select())
    {
        qDebug() << "Ошибка обновления модели операций:" << lastError().text();
        return false;
    }

    qDebug() << "Операция успешно добавлена, ID:" << query->lastInsertId().toInt();
    return true;
}

bool OperationModel::updateOperation(int id, const QString& articleName, double debit, double credit, const QDateTime& date)
{
    int articleId = getArticleIdByName(articleName);
    if (articleId == -1)
    {
        qDebug() << "Статья с именем" << articleName << "не найдена";
        return false;
    }

    QDateTime localDate = date.toLocalTime();
    QString dateStr = localDate.toString("yyyy-MM-dd HH:mm");
    query->prepare("UPDATE operations SET article_id = :article_id, debit = :debit, credit = :credit, create_date = :create_date WHERE id = :id");
    query->bindValue(":article_id", articleId);
    query->bindValue(":debit", debit);
    query->bindValue(":credit", credit);
    query->bindValue(":create_date", dateStr);
    query->bindValue(":id", id);

    if (!query->exec())
    {
        qDebug() << "Ошибка обновления операции:" << query->lastError().text();
        return false;
    }

    if (!select())
    {
        qDebug() << "Ошибка обновления модели:" << lastError().text();
        return false;
    }

    qDebug() << "Операция успешно обновлена";
    return true;
}

bool OperationModel::deleteOperation(int id)
{
    query->prepare("DELETE FROM operations WHERE id = :id");
    query->bindValue(":id", id);
    if (!query->exec())
    {
        qDebug() << "Ошибка удаления операции:" << query->lastError().text();
        return false;
    }

    if (!select())
    {
        qDebug() << "Ошибка обновления модели:" << lastError().text();
        return false;
    }

    qDebug() << "Операция успешно удалена";
    return true;
}

void OperationModel::setDateFilter(const QDateTime& startDate, const QDateTime& endDate)
{
    QString filter = QString("create_date >= '%1' AND create_date <= '%2'")
                            .arg(startDate.toString("yyyy-MM-dd hh:mm:ss"))
                            .arg(endDate.toString("yyyy-MM-dd hh:mm:ss"));
    setFilter(filter);
    select();
}

void OperationModel::setArticleFilter(int articleId)
{
    QString filter = QString("article_id = %1").arg(articleId);
    setFilter(filter);
    select();
}

void OperationModel::clearFilters()
{
    setFilter("");
    select();
}

int OperationModel::getArticleIdByName(const QString& name) const
{
    query->prepare("SELECT id FROM articles WHERE name = :name");
    query->bindValue(":name", name);

    if (query->exec() && query->next())
    {
        return query->value(0).toInt();
    }
    qDebug() << "Ошибка SQL при добавлении операции:" << query->lastError().text();
    return -1;
}

QString OperationModel::getQueryError() const
{
    return query->lastError().databaseText();
}


double OperationModel::getTotalDebit() const
{
    double total = 0;
    for (int i = 0; i < rowCount(); ++i)
    {
        total += data(index(i, fieldIndex("debit"))).toDouble();
    }
    return total;
}

double OperationModel::getTotalCredit() const
{
    double total = 0;
    for (int i = 0; i < rowCount(); ++i)
    {
        total += data(index(i, fieldIndex("credit"))).toDouble();
    }
    return total;
}

double OperationModel::getTotalProfit() const
{
    return getTotalDebit() - getTotalCredit();
}
