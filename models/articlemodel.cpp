#include "articlemodel.h"

#include <QSqlRecord>
#include <QSqlError>

ArticleModel::ArticleModel(QObject* parent, QSqlDatabase db):
    QSqlTableModel(parent, db)
{
    setTable("articles");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    select();

    query = new QSqlQuery(db);
}

bool ArticleModel::addArticle(const QString& name)
{
    QString formattedName = name.trimmed().toLower();
    if (formattedName.isEmpty())
    {
        return false;
    }
    formattedName[0] = formattedName[0].toUpper();

    if (articleExists(formattedName))
    {
        qDebug() << "Статья с именем" << formattedName << "уже существует";
        return false;
    }

    query->prepare("INSERT INTO articles (name) VALUES (:name)");
    query->bindValue(":name", formattedName);

    if (!query->exec())
    {
        qDebug() << "Ошибка SQL при добавлении статьи:" << query->lastError().text();
        return false;
    }

    return select();
}

bool ArticleModel::updateArticle(int id, const QString& newName)
{
    if (newName.trimmed().isEmpty())
    {
        return false;
    }

    query->prepare("UPDATE articles SET name = :name WHERE id = :id");
    query->bindValue(":name", newName.trimmed());
    query->bindValue(":id", id);

    if (!query->exec())
    {
        qDebug() << "Ошибка обновления статьи:" << query->lastError().text();
        return false;
    }

    return select();
}

bool ArticleModel::deleteArticle(int id)
{
    query->prepare("DELETE FROM articles WHERE id = :id");
    query->bindValue(":id", id);

    if (!query->exec())
    {
        qDebug() << "Ошибка удаления статьи:" << query->lastError().text();
        return false;
    }

    return select();
}

bool ArticleModel::articleExists(const QString& name) const
{
    query->prepare("SELECT COUNT(*) FROM articles WHERE name = :name");
    query->bindValue(":name", name);

    if (query->exec() && query->next())
    {
        return query->value(0).toInt() > 0;
    }
    return false;
}

QString ArticleModel::getArticleName(int id) const
{
    query->prepare("SELECT name FROM articles WHERE id = :id");
    query->bindValue(":id", id);

    if (query->exec() && query->next())
    {
        return query->value(0).toString();
    }

    return "Неизвестная статья";
}

QMap<int, QString> ArticleModel::getAllArticles() const
{
    QMap<int, QString> articles;
    QSqlQuery *query = new QSqlQuery(this->database());
    query->exec("SELECT id, name FROM articles ORDER BY name");

    while (query->next())
    {
        articles.insert(query->value(fieldIndex("id")).toInt(), query->value(fieldIndex("name")).toString());
    }
    return articles;
}

bool ArticleModel::articleHasOperations(int articleId) const
{
    query->prepare("SELECT COUNT(*) FROM operations WHERE article_id = :article_id");
    query->bindValue(":article_id", articleId);

    if (query->exec() && query->next())
    {
        return query->value(0).toInt() > 0;
    }

    return false;
}

QString ArticleModel::getQueryError() const
{
    return query->lastError().databaseText();
}
