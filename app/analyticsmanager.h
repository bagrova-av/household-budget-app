#ifndef ANALYTICSMANAGER_H
#define ANALYTICSMANAGER_H

#include <QDateTime>
#include <QList>
#include <QVector>
#include <QString>
#include <QSqlQuery>

struct FinancialDynamicsData
{
    QDate date;
    double debit;
    double credit;
    double profit;
};

struct ArticlePercentData
{
    int articleId;
    QString articleName;
    double sum;
    double percent;
};

struct ProfitData
{
    int balanceId;
    QDate periodStart;
    QDate periodEnd;
    double profit;
};

class AnalyticsManager : public QObject
{
    Q_OBJECT

private:
    QSqlQuery *query;
    QString intVectorToPostgresArray(const QVector<int>& vector) const;

public:
    explicit AnalyticsManager(QSqlDatabase db, QObject *parent = nullptr);

    QVector<FinancialDynamicsData> getFinancialDynamics(const QDateTime& startDate, const QDateTime& endDate,
                                              const QList<int>& articleIds);

    QVector<ArticlePercentData> getArticlePercent(const QDateTime& startDate, const QDateTime& endDate,
                                              const QList<int>& articleIds,
                                              const QString& flowType);

    QVector<ProfitData> getProfitOverTime(const QDateTime& startDate, const QDateTime& endDate);
};

#endif // ANALYTICSMANAGER_H
