#ifndef ARTICLEMODEL_H
#define ARTICLEMODEL_H

#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>

class ArticleModel : public QSqlTableModel
{
    Q_OBJECT

private:
    QSqlQuery *query;

public:
    explicit ArticleModel(QObject* parent = nullptr, QSqlDatabase db = QSqlDatabase());

    bool addArticle(const QString& name);
    bool updateArticle(int id, const QString& newName);
    bool deleteArticle(int id);

    bool articleExists(const QString& name) const;

    QString getArticleName(int id) const;
    QMap<int, QString> getAllArticles() const;
    bool articleHasOperations(int articleId) const;

    QString getQueryError() const;
};

#endif // ARTICLEMODEL_H
