#include "analyticsmanager.h"

#include <QDebug>
#include <QSqlError>

AnalyticsManager::AnalyticsManager(QSqlDatabase db, QObject *parent) :
    QObject(parent)
{
    query = new QSqlQuery(db);
}

QVector<FinancialDynamicsData> AnalyticsManager::getFinancialDynamics(const QDateTime& startDate, const QDateTime& endDate,
                                                                            const QList<int>& articleIds)
{
    QVector<FinancialDynamicsData> result;

    QString arrayStr = intVectorToPostgresArray(articleIds);
    QString sql = QString("SELECT * FROM calculate_financial_dynamics(:start_date, :end_date, %1)").arg(arrayStr);
    query->prepare(sql);
    query->bindValue(":start_date", startDate);
    query->bindValue(":end_date", endDate);
    query->bindValue(":articles_ids", arrayStr);

    if (query->exec())
    {
        while (query->next())
        {
            FinancialDynamicsData data;
            data.date = query->value("operation_date").toDate();
            data.debit = query->value("total_debit").toDouble();
            data.credit = query->value("total_credit").toDouble();
            data.profit = query->value("total_profit").toDouble();
            result.push_back(data);
        }
    }
    else
    {
        qDebug() << "Ошибка выполнения calculate_financial_dynamics:" << query->lastError().text();
    }

    return result;
}

QVector<ArticlePercentData> AnalyticsManager::getArticlePercent(const QDateTime& startDate, const QDateTime& endDate,
                                                                         const QList<int>& articleIds,
                                                                         const QString& flowType)
{
    QVector<ArticlePercentData> result;

    QString arrayStr = intVectorToPostgresArray(articleIds);
    QString sql = QString("SELECT * FROM calculate_percent_by_articles(:start_date, :end_date, %1, :flow_type)").arg(arrayStr);
    query->prepare(sql);
    query->bindValue(":start_date", startDate);
    query->bindValue(":end_date", endDate);
    query->bindValue(":articles_ids", arrayStr);
    query->bindValue(":flow_type", flowType);

    if (query->exec())
    {
        while (query->next())
        {
            ArticlePercentData data;
            data.articleId = query->value("article_id").toInt();
            data.articleName = query->value("article_name").toString();
            data.sum = query->value("article_sum").toDouble();
            data.percent = query->value("article_percent").toDouble();
            result.push_back(data);
        }
    }
    else
    {
        qDebug() << "Ошибка выполнения calculate_percent_by_articles:" << query->lastError().text();
    }

    return result;
}

QVector<ProfitData> AnalyticsManager::getProfitOverTime(const QDateTime& startDate, const QDateTime& endDate)
{
    QVector<ProfitData> result;

    query->prepare("SELECT * FROM calculate_profit(:start_date, :end_date)");
    query->bindValue(":start_date", startDate);
    query->bindValue(":end_date", endDate);

    if (query->exec())
    {
        while (query->next())
        {
            ProfitData data;
            data.balanceId = query->value("balance_id").toInt();
            data.periodStart = query->value("period_start").toDate();
            data.periodEnd = query->value("period_end").toDate();
            data.profit = query->value("profit").toDouble();
            result.push_back(data);
        }
    }
    else
    {
        qDebug() << "Ошибка выполнения calculate_profit:" << query->lastError().text();
    }

    return result;
}

QString AnalyticsManager::intVectorToPostgresArray(const QVector<int>& vector) const
{
    if (vector.isEmpty())
    {
        return "'{}'::integer[]";
    }

    QString arrayStr = "'{";
    for (int i = 0; i < vector.size(); ++i)
    {
        if (i > 0)
        {
            arrayStr += ",";
        }
        arrayStr += QString::number(vector[i]);
    }
    arrayStr += "}'::integer[]";

    return arrayStr;
}
