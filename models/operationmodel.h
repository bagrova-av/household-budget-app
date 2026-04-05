#ifndef OPERATIONMODEL_H
#define OPERATIONMODEL_H

#include <QSqlDatabase>
#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

class OperationModel : public QSqlRelationalTableModel
{
    Q_OBJECT

private:
    QSqlQuery *query;

public:
    explicit OperationModel(QObject* parent = nullptr, QSqlDatabase db = QSqlDatabase());
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool addOperation(const QString& articleName, double debit, double credit, const QDateTime& date);
    bool updateOperation(int id, const QString& articleName, double debit, double credit, const QDateTime& date);
    bool deleteOperation(int id);

    void setDateFilter(const QDateTime& startDate, const QDateTime& endDate);
    void setArticleFilter(int articleId);
    void clearFilters();

    int getArticleIdByName(const QString& name) const;
    QString getQueryError() const;

    double getTotalDebit() const;
    double getTotalCredit() const;
    double getTotalProfit() const;
};

#endif // OPERATIONMODEL_H
