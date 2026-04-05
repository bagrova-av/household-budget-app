#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QInputDialog>
#include <QDebug>
#include <QSqlError>
#include <QModelIndexList>
#include <QModelIndex>
#include <QMessageBox>
#include <QList>
#include <QLineSeries>

#include <QDateTimeAxis>
#include <QValueAxis>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QStackedBarSeries>

#include <QFileDialog>
#include <QTextDocument>
#include <QPrinter>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include "operationdialog.h"

MainWindow::MainWindow(DatabaseManager* dbManager, const QString& currentUserRole, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    userRole(currentUserRole),
    databaseManager(dbManager)
{
    ui->setupUi(this);
    analyticsManager = new AnalyticsManager(dbManager->getDatabase(), this);

    // Инициализируем модели
    articleModel = new ArticleModel(this, dbManager->getDatabase());
    operationModel = new OperationModel(this, dbManager->getDatabase());
    balanceModel = new BalanceModel(this, dbManager->getDatabase());

    ui->articlesTableView->setModel(articleModel);
    ui->operationsTableView->setModel(operationModel);
    ui->balancesTableView->setModel(balanceModel);

    if (userRole != "admin")
    {
        // Отключаем кнопки создания, редактирования и удаления
        ui->addOperationButton->setEnabled(false);
        ui->editOperationButton->setEnabled(false);
        ui->deleteOperationButton->setEnabled(false);
        ui->addArticleButton->setEnabled(false);
        ui->deleteArticleButton->setEnabled(false);
        ui->createBalanceButton->setEnabled(false);
        ui->disbandBalanceButton->setEnabled(false);
    }

    ui->balanceDateStartEdit->setDateTime(QDateTime::currentDateTime());
    ui->balanceDateEndEdit->setDateTime(QDateTime::currentDateTime());
    ui->analysisStartDateEdit->setDateTime(QDateTime::currentDateTime());
    ui->analysisEndDateEdit->setDateTime(QDateTime::currentDateTime());

    ui->articlesListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    QSqlQuery query("SELECT id, name FROM articles");
    while (query.next())
    {
        QListWidgetItem *item = new QListWidgetItem(query.value("name").toString());
        item->setData(Qt::UserRole, query.value("id").toInt());
        ui->articlesListWidget->addItem(item);
    }
    ui->flowTypeComboBox->clear();
    ui->flowTypeComboBox->addItem("Приход", "приход");
    ui->flowTypeComboBox->addItem("Расход", "расход");

    ui->operationsTableView->horizontalHeader()->setSectionResizeMode(operationModel->fieldIndex("create_date"),
                                                                      QHeaderView::Stretch);
    ui->operationsTableView->horizontalHeader()->setSectionResizeMode(operationModel->fieldIndex("name"),
                                                                      QHeaderView::Stretch);
    ui->operationsTableView->horizontalHeader()->setSectionResizeMode(operationModel->fieldIndex("debit"),
                                                                      QHeaderView::Stretch);
    ui->operationsTableView->horizontalHeader()->setSectionResizeMode(operationModel->fieldIndex("credit"),
                                                                      QHeaderView::Stretch);

    ui->balancesTableView->horizontalHeader()->setSectionResizeMode(balanceModel->fieldIndex("create_date"),
                                                                      QHeaderView::Stretch);
    ui->balancesTableView->horizontalHeader()->setSectionResizeMode(balanceModel->fieldIndex("amount"),
                                                                      QHeaderView::Stretch);
    ui->balancesTableView->horizontalHeader()->setSectionResizeMode(balanceModel->fieldIndex("debit"),
                                                                      QHeaderView::Stretch);
    ui->balancesTableView->horizontalHeader()->setSectionResizeMode(balanceModel->fieldIndex("credit"),
                                                                      QHeaderView::Stretch);

    ui->articlesTableView->horizontalHeader()->setSectionResizeMode(articleModel->fieldIndex("name"),
                                                                    QHeaderView::Stretch);

    ui->chartDynamicsView->setRenderHint(QPainter::Antialiasing);
    ui->chartDynamicsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->chartPercentView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->chartPercentView->setRenderHint(QPainter::Antialiasing);
    ui->chartProfitView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->chartProfitView->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addOperationButton_clicked()
{
    QMap<int, QString> articles = articleModel->getAllArticles();
    if (articles.isEmpty())
    {
        showError("Нет доступных статей. Сначала создайте хотя бы одну статью.");
        return;
    }

    AddOperationDialog dialog(articles, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString articleName = dialog.getArticleName();
        double debit = dialog.getDebit();
        double credit = dialog.getCredit();
        QDateTime operationDate = dialog.getOperationDate();

        qDebug() << "Добавление операции: статья =" << articleName
                 << "приход =" << debit << "расход =" << credit
                 << "дата =" << operationDate.toString("dd.MM.yyyy hh:mm:ss");

        if (operationModel->addOperation(articleName, debit, credit, operationDate))
        {
            showInfo("Операция успешно добавлена");
            qDebug() << "Операция успешно добавлена в БД";
        }
        else
        {
            showError("Не удалось добавить операцию.\n\n" + operationModel->getQueryError());
            qDebug() << "Ошибка при добавлении операции" << operationModel->lastError().text();
        }
    }
}


void MainWindow::on_editOperationButton_clicked()
{
    QModelIndex currentIndex = ui->operationsTableView->currentIndex();
    if (!currentIndex.isValid())
    {
        showError("Выберите операцию для редактирования");
        return;
    }

    int row = currentIndex.row();

    qDebug() << "=== ДАННЫЕ ВЫБРАННОЙ СТРОКИ ===";
    for (int col = 0; col < operationModel->columnCount(); ++col)
    {
        QString columnName = operationModel->headerData(col, Qt::Horizontal).toString();
        QVariant data = operationModel->data(operationModel->index(row, col));
        qDebug() << "Столбец" << col << "(" << columnName << "):" << data.toString();
    }
    qDebug() << "=== КОНЕЦ ДАННЫХ ===";

    int operationId = operationModel->data(operationModel->index(row, operationModel->fieldIndex("id"))).toInt();

    // Получаем текущие данные операции
    QString currentArticleName = operationModel->data(operationModel->index(row, operationModel->fieldIndex("name"))).toString();
    double currentDebit = operationModel->data(operationModel->index(row, operationModel->fieldIndex("debit"))).toDouble();
    double currentCredit = operationModel->data(operationModel->index(row, operationModel->fieldIndex("credit"))).toDouble();
    QDateTime currentDate = operationModel->data(operationModel->index(row, operationModel->fieldIndex("create_date"))).toDateTime();

    qDebug() << "Редактирование операции ID:" << operationId;

    QMap<int, QString> articles = articleModel->getAllArticles();
    if (articles.isEmpty())
    {
        showError("Нет доступных статей");
        return;
    }

    AddOperationDialog dialog(articles, this);
    dialog.setInitialValues(currentArticleName, currentDebit, currentCredit, currentDate);
    if (dialog.exec() == QDialog::Accepted)
    {
        // Получаем новые данные из диалога
        QString newArticleName = dialog.getArticleName();
        double newDebit = dialog.getDebit();
        double newCredit = dialog.getCredit();
        QDateTime newDate = dialog.getOperationDate();

        qDebug() << "Новые данные - Статья:" << newArticleName
                 << "Приход:" << newDebit << "Расход:" << newCredit
                 << "Дата:" << newDate.toString("dd.MM.yyyy hh:mm:ss");

        if (operationModel->updateOperation(operationId, newArticleName, newDebit, newCredit, newDate))
        {
            showInfo("Операция успешно обновлена");
            qDebug() << "Операция успешно обновлена в БД";
        }
        else
        {
            showError("Не удалось обновить операцию.\n\n" + operationModel->getQueryError());
            qDebug() << "Ошибка при обновлении операции:" << operationModel->getQueryError();
        }
    }
    else
    {
        qDebug() << "Пользователь отменил редактирование операции";
    }
}


void MainWindow::on_deleteOperationButton_clicked()
{
    QModelIndex currentIndex = ui->operationsTableView->currentIndex();

    if (!currentIndex.isValid())
    {
        showError("Выберите операцию для удаления");
        return;
    }

    int row = currentIndex.row();

    qDebug() << "=== ДАННЫЕ ВЫБРАННОЙ СТРОКИ ===";
    for (int col = 0; col < operationModel->columnCount(); ++col)
    {
        QString columnName = operationModel->headerData(col, Qt::Horizontal).toString();
        QVariant data = operationModel->data(operationModel->index(row, col));
        qDebug() << "Столбец" << col << "(" << columnName << "):" << data.toString();
    }
    qDebug() << "=== КОНЕЦ ДАННЫХ ===";

    // Получаем ID выбранной операции
    int operationId = operationModel->data(operationModel->index(row, operationModel->fieldIndex("id"))).toInt();

    // Получаем данные операции для информационного сообщения
    QString articleName = operationModel->data(operationModel->index(row, operationModel->fieldIndex("name"))).toString();
    double debit = operationModel->data(operationModel->index(row, operationModel->fieldIndex("debit"))).toDouble();
    double credit = operationModel->data(operationModel->index(row, operationModel->fieldIndex("credit"))).toDouble();
    QDateTime date = operationModel->data(operationModel->index(row, operationModel->fieldIndex("create_date"))).toDateTime();
    QVariant balanceId = operationModel->data(operationModel->index(row, operationModel->fieldIndex("balance_id")));

    qDebug() << "Попытка удаления операции ID:" << operationId;
    qDebug() << "Данные операции - Статья:" << articleName
             << "Приход:" << debit << "Расход:" << credit
             << "Дата:" << date.toString("dd.MM.yyyy hh:mm:ss")
             << "Баланс ID:" << (balanceId.isNull() ? "NULL" : balanceId.toString());

    QString message = QString("Вы уверены, что хотите удалить операцию?\n\n"
                              "id: %1\n"
                              "Приход: %2 ₽\n"
                              "Расход: %3 ₽\n"
                              "Статья: %4\n"
                              "Дата: %5")
                          .arg(operationId)
                          .arg(debit)
                          .arg(credit)
                          .arg(articleName)
                          .arg(date.toString("dd.MM.yyyy hh:mm:ss"));

    // Запрашиваем подтверждение у пользователя
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение удаления",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No // По умолчанию кнопка "Нет"
        );

    if (reply == QMessageBox::Yes)
    {
        qDebug() << "Пользователь подтвердил удаление операции ID:" << operationId;
        if (operationModel->deleteOperation(operationId))
        {
            showInfo("Операция успешно удалена");
            qDebug() << "Операция успешно удалена из БД";

            ui->operationsTableView->viewport()->update();
        }
        else
        {
            showError("Не удалось удалить операцию.\n\n" + operationModel->getQueryError());
            qDebug() << "Ошибка при удалении операции:" << operationModel->getQueryError();
        }
    }
    else
    {
        qDebug() << "Пользователь отменил удаление операции";
    }
}


void MainWindow::on_createBalanceButton_clicked()
{
    QDate startDate = ui->balanceDateStartEdit->date();
    QDate endDate = ui->balanceDateEndEdit->date();

    qDebug() << "Попытка формирования баланса за период:"
             << startDate.toString("dd.MM.yyyy") << "-" << endDate.toString("dd.MM.yyyy");

    QString message = QString("Сформировать баланс за период:\n"
                              "с %1 по %2?\n\n"
                              "После формирования операции за этот период\n"
                              "будут заблокированы для редактирования.")
                          .arg(startDate.toString("dd.MM.yyyy"))
                          .arg(endDate.toString("dd.MM.yyyy"));

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение формирования баланса",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (reply == QMessageBox::Yes)
    {
        QDateTime createDate = QDateTime::currentDateTime();
        QDateTime periodStart(startDate, QTime(0,0,0));
        QDateTime periodEnd(endDate, QTime(23,59,59));

        if (balanceModel->createBalance(createDate, periodStart, periodEnd))
        {
            showInfo("Баланс успешно сформирован");
            qDebug() << "Баланс успешно создан за период"
                     << startDate.toString("dd.MM.yyyy") << "-" << endDate.toString("dd.MM.yyyy");

            // Обновляем таблицу операций чтобы показать изменения
            operationModel->select();
        }
        else
        {
            showError("Не удалось сформировать баланс.\n\n"+balanceModel->getQueryError());
            qDebug() << "Ошибка формирования баланса:" << balanceModel->lastError().text();
        }
    }
}


void MainWindow::on_disbandBalanceButton_clicked()
{
    QModelIndex currentIndex = ui->balancesTableView->currentIndex();

    if (!currentIndex.isValid())
    {
        showError("Выберите баланс для расформирования");
        return;
    }

    int row = currentIndex.row();

    // Получаем данные выбранного баланса
    int balanceId = balanceModel->data(balanceModel->index(row, balanceModel->fieldIndex("id"))).toInt();
    QDate periodStart = balanceModel->data(balanceModel->index(row, balanceModel->fieldIndex("period_start"))).toDate();
    QDate periodEnd = balanceModel->data(balanceModel->index(row, balanceModel->fieldIndex("period_end"))).toDate();
    double debit = balanceModel->data(balanceModel->index(row, balanceModel->fieldIndex("debit"))).toDouble();
    double credit = balanceModel->data(balanceModel->index(row, balanceModel->fieldIndex("credit"))).toDouble();
    double amount = balanceModel->data(balanceModel->index(row, balanceModel->fieldIndex("amount"))).toDouble();

    qDebug() << "Попытка расформирования баланса ID:" << balanceId;

    QString message = QString("Расформировать баланс?\n\n"
                              "Период: %1 - %2\n"
                              "Приход: %3 ₽\n"
                              "Расход: %4 ₽\n"
                              "Прибыль: %5 ₽\n\n"
                              "После расформирования операции за этот период\n"
                              "станут доступны для редактирования.")
                          .arg(periodStart.toString("dd.MM.yyyy"))
                          .arg(periodEnd.toString("dd.MM.yyyy"))
                          .arg(debit, 0, 'f', 2)
                          .arg(credit, 0, 'f', 2)
                          .arg(amount, 0, 'f', 2);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение расформирования баланса",
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (reply == QMessageBox::Yes)
    {
        if (balanceModel->disbandBalance(balanceId))
        {
            showInfo("Баланс успешно расформирован");
            qDebug() << "Баланс ID" << balanceId << "успешно расформирован";
            operationModel->select();
        }
        else
        {
            showError("Не удалось расформировать баланс.\n\n" + balanceModel->getQueryError());
            qDebug() << "Ошибка расформирования баланса:" << balanceModel->lastError().text();
        }
    }
}


void MainWindow::on_addArticleButton_clicked()
{
    bool ok;
    QString articleName = QInputDialog::getText(this, "Добавить статью",
                                                "Введите название статьи:",
                                                QLineEdit::Normal, "", &ok);
    if (ok && !articleName.trimmed().isEmpty())
    {
        if (articleModel->addArticle(articleName))
        {
            qDebug() << "Статья успешно добавлена";
            showInfo("Статья успешно добавлена");
            refreshArticlesList();
        }
        else
        {
            qDebug() << "Не удалось добавить статью";
            showError("Не удалось добавить статью.\n\n" + articleModel->getQueryError());
        }
    }
}


void MainWindow::on_deleteArticleButton_clicked()
{
    QModelIndex currentIndex = ui->articlesTableView->currentIndex();

    if (!currentIndex.isValid())
    {
        QMessageBox::information(this, "Удаление статьи", "Выберите статью для удаления");
        return;
    }

    int row = currentIndex.row();
    int articleId = articleModel->data(articleModel->index(row, articleModel->fieldIndex("id"))).toInt();
    QString articleName = articleModel->data(articleModel->index(row, articleModel->fieldIndex("name"))).toString();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение удаления",
                                                              QString("Вы уверены, что хотите удалить статью \"%1\"?").arg(articleName));

    if (reply == QMessageBox::Yes)
    {
        if (articleModel->deleteArticle(articleId))
        {
            qDebug() << "Статья успешно удалена";
            showInfo("Статья успешно удалена");
            refreshArticlesList();
        }
        else
        {
            showError("Не удалось удалить статью.\n\n" + articleModel->getQueryError());
        }
    }
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Ошибка", message);
}

void MainWindow::showInfo(const QString& message)
{
    QMessageBox::information(this, "Информация", message);
}


void MainWindow::generateDynamicsChart(const QDateTime& startDate, const QDateTime& endDate,
                                       const QVector<int>& articleIds)
{
    auto dynamics = analyticsManager->getFinancialDynamics(startDate, endDate, articleIds);

    qDebug() << "dynamics size:" << dynamics.size();
    qDebug() << "=== FINANCIAL DYNAMICS ===";
    qDebug() << "start:" << startDate << "end:" << endDate;
    qDebug() << "articles:" << articleIds;

    qDebug() << "rows:" << dynamics.size();
    for (const auto& p : dynamics)
    {
        qDebug() << p.date.toString("yyyy-MM-dd")
        << "debit=" << p.debit
        << "credit=" << p.credit
        << "profit=" << p.profit;
    }

    QLineSeries *debitSeries = new QLineSeries();
    debitSeries->setName("Расход");
    QLineSeries *creditSeries = new QLineSeries();
    creditSeries->setName("Доход");
    QLineSeries *profitSeries = new QLineSeries();
    profitSeries->setName("Прибыль");
    for (const auto& point : dynamics)
    {
        auto ms = point.date.startOfDay().toMSecsSinceEpoch();
        debitSeries->append(ms, point.debit);
        creditSeries->append(ms, point.credit);
        profitSeries->append(ms, point.profit);
    }

    QChart *chart = new QChart();
    chart->addSeries(debitSeries);
    chart->addSeries(creditSeries);
    chart->addSeries(profitSeries);
    chart->setTitle("Динамика доходов и расходов");

    QDateTimeAxis* axisX = new QDateTimeAxis();
    axisX->setFormat("dd.MM.yyyy");
    axisX->setTitleText("Дата");
    axisX->setRange(startDate, endDate);
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Сумма (руб)");
    chart->addAxis(axisY, Qt::AlignLeft);

    debitSeries->attachAxis(axisX);
    debitSeries->attachAxis(axisY);
    creditSeries->attachAxis(axisX);
    creditSeries->attachAxis(axisY);
    profitSeries->attachAxis(axisX);
    profitSeries->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    ui->chartDynamicsView->setChart(chart);
    ui->chartDynamicsView->update();
    ui->chartDynamicsView->repaint();
}
void MainWindow::generateDynamicsBarChart(const QDateTime& startDate, const QDateTime& endDate,
                                                 const QVector<int>& articleIds)
{
    auto dynamics = analyticsManager->getFinancialDynamics(startDate, endDate, articleIds);

    QChart *chart = new QChart();
    chart->setTitle("Динамика доходов и расходов");

    QVector<FinancialDynamicsData> filteredDynamics;
    for (const auto& point : dynamics)
    {
        if (point.debit > 0 || point.credit > 0)
        {
            filteredDynamics.push_back(point);
        }
    }

    if (filteredDynamics.isEmpty())
    {
        chart->setTitle("Нет операций за выбранный период");
        ui->chartDynamicsView->setChart(chart);
        return;
    }

    // Сортируем по дате
    std::sort(filteredDynamics.begin(), filteredDynamics.end(),
              [](const auto& a, const auto& b){ return a.date < b.date; });

    auto *debitSet  = new QBarSet("Расход");
    auto *creditSet = new QBarSet("Доход");

    QStringList categories;

    for (const auto& row : filteredDynamics)
    {
        categories << row.date.toString("dd.MM.yyyy");
        *debitSet  << row.debit;
        *creditSet << row.credit;
    }

    // Группы
    auto *series = new QBarSeries();
    series->append(debitSet);
    series->append(creditSet);

    chart->addSeries(series);

    // Настройка оси X
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Дата");
    axisX->setLabelsAngle(-90); // -90 градусов = вертикально вниз
    axisX->setLabelsFont(QFont("Arial", 8));

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Настройка оси Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Сумма (руб)");
    axisY->setLabelFormat("%.0f");

    // Автоматически определяем диапазон значений Y
    double maxY = 0;
    for (const auto& row : filteredDynamics)
    {
        double dailyTotal = row.debit + row.credit;
        if (dailyTotal > maxY)
        {
            maxY = dailyTotal;
        }
    }
    axisY->setRange(0, maxY > 0 ? maxY * 1.1 : 1000);

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Анимация
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Настройка легенды
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    ui->chartDynamicsView->setChart(chart);

    qDebug() << "Stacked bar chart shows" << filteredDynamics.size()
             << "days with operations out of" << dynamics.size() << "total days in range";
}
void MainWindow::generatePieChart(const QDateTime& startDate, const QDateTime& endDate,
                      const QVector<int>& articleIds,
                      const QString& flowType)
{
    auto percents = analyticsManager->getArticlePercent(startDate, endDate, articleIds, flowType);
    qDebug() << "=== PERCENT ===";
    qDebug() << "rows:" << percents.size();
    for (const auto& p : percents)
    {
        qDebug() << p.articleName << p.sum << p.percent;
    }

    QPieSeries *pie = new QPieSeries();
    double total = 0;
    for (const auto& p : percents)
    {
        total += p.sum;
    }
    if (total == 0.0)
    {
        QChart *chart = new QChart();
        chart->setTitle("Нет данных для построения диаграммы");
        ui->chartPercentView->setChart(chart);
        return;
    }

    for (const auto& point : percents)
    {
        QPieSlice *slice = pie->append(point.articleName + QString(" (%1%)").arg(point.percent, 0, 'f', 2), point.sum);
        if (point.percent >= 1)
        {
            slice->setLabelVisible(true);
        }
    }

    QChart *chart = new QChart();
    chart->addSeries(pie);
    chart->setTitle(QString("Распределение %1 по статьям").arg(flowType+'a'));

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    ui->chartPercentView->setChart(chart);
    ui->chartPercentView->update();
    ui->chartPercentView->repaint();
}
void MainWindow::generateProfitChart(const QDateTime& startDate, const QDateTime& endDate)
{
    auto profits = analyticsManager->getProfitOverTime(startDate, endDate);
    qDebug() << "=== PROFIT ===";
    qDebug() << "rows:" << profits.size();
    for (const auto& p : profits)
    {
        qDebug() << p.periodStart.toString("yyyy-MM-dd") << " " << p.periodEnd.toString("yyyy-MM-dd")
        << p.profit;
    }

    QLineSeries *profitLine = new QLineSeries();
    profitLine->setName("Чистая прибыль");

    double minProfit = 0.0, maxProfit = 0.0;
    bool firstPoint = true;

    for (const auto& point : profits)
    {
        qint64 ms = point.periodEnd.startOfDay().toMSecsSinceEpoch();
        profitLine->append(QPointF(ms, point.profit));

        if (firstPoint)
        {
            minProfit = maxProfit = point.profit;
            firstPoint = false;
        }
        else
        {
            if (point.profit < minProfit)
            {
                minProfit = point.profit;
            }
            if (point.profit > maxProfit)
            {
                maxProfit = point.profit;
            }
        }
    }

    if (profits.isEmpty())
    {
        qint64 msStart = startDate.date().startOfDay().toMSecsSinceEpoch();
        profitLine->append(QPointF(msStart, 0));
        minProfit = 0;
        maxProfit = 1;
    }

    // Если min == max, расширяем диапазон для оси Y
    if (qFuzzyCompare(minProfit, maxProfit))
    {
        minProfit -= 1.0;
        maxProfit += 1.0;
    }

    QChart *chart = new QChart();
    chart->addSeries(profitLine);
    chart->setTitle("Чистая прибыль бюджета от времени");
     chart->legend()->setVisible(false);

    // Настройка оси X
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("dd.MM.yyyy");
    axisX->setTitleText("Период");
    axisX->setRange(startDate, endDate);
    chart->addAxis(axisX, Qt::AlignBottom);
    axisX->setLabelsAngle(-90);
    axisX->setLabelsFont(QFont("Arial", 8));
    profitLine->attachAxis(axisX);

    // Настройка оси Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Сумма (руб)");
    axisY->setRange(minProfit * 0.9, maxProfit * 1.1);
    chart->addAxis(axisY, Qt::AlignLeft);
    profitLine->attachAxis(axisY);

    profitLine->setPointsVisible(true);
    profitLine->setPointLabelsVisible(true);
    profitLine->setPointLabelsFormat("@yPoint");
    profitLine->setPointLabelsClipping(false);

    ui->chartProfitView->setChart(chart);
}

void MainWindow::on_updateChartsButton_clicked()
{
    QDateTime startDate = ui->analysisStartDateEdit->dateTime();
    startDate.setTime(QTime(0,0,0));
    QDateTime endDate = ui->analysisEndDateEdit->dateTime();
    endDate.setTime(QTime(23,59,59));
    if (startDate > endDate)
    {
        showError("Дата начала не может быть позже даты окончания");
        return;
    }

    QList<int> articleIds;
    for (auto item : ui->articlesListWidget->selectedItems())
    {
        articleIds.append(item->data(Qt::UserRole).toInt());
    }
    if (articleIds.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы одну статью.");
        return;
    }

    generateDynamicsChart(startDate, endDate, articleIds);
    generateDynamicsBarChart(startDate, endDate, articleIds);

    QString flowType = ui->flowTypeComboBox->currentData().toString();
    qDebug() << flowType;
    generatePieChart(startDate, endDate, articleIds, flowType);

    generateProfitChart(startDate, endDate);
}

void MainWindow::refreshArticlesList()
{
    ui->articlesListWidget->clear();

    QSqlQuery query(databaseManager->getDatabase());
    query.prepare("SELECT id, name FROM articles ORDER BY name");
    if (!query.exec())
    {
        qDebug() << "Ошибка при обновлении списка статей:" << query.lastError().text();
        return;
    }

    while (query.next())
    {
        QListWidgetItem *item = new QListWidgetItem(query.value("name").toString());
        item->setData(Qt::UserRole, query.value("id").toInt());
        ui->articlesListWidget->addItem(item);
    }
}

void MainWindow::on_actionExportPDF_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save PDF Report", QString(), "PDF Files (*.pdf)");
    if (filename.isEmpty())
    {
        return;
    }

    if (!filename.endsWith(".pdf", Qt::CaseInsensitive))
    {
        filename += ".pdf";
    }

    QSqlDatabase db = databaseManager->getDatabase();
    QSqlQuery query(db);

    // Собираем HTML-строку
    QString html;
    html += "<html><head><meta charset='utf-8'><style>"
            "body { font-family: Arial, sans-serif; }"
            "h1 { font-size: 16pt; }"
            "h2 { font-size: 12pt; }"
            "table { border-collapse: collapse; width: 100%; margin-bottom: 15px; }"
            "th, td { border: 1px solid #444; padding: 6px; font-size: 10pt; }"
            "th { background: #eee; }"
            ".balance { margin-top: 15px; page-break-inside: avoid; }"
            "</style></head><body>";
    html += QString("<h1>Отчёт домашнего бюджета</h1>");
    html += QString("<p>Дата формирования: %1</p>").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));

    // Выбираем балансы
    if (!query.exec("SELECT id, create_date, period_start, period_end, debit, credit, amount FROM balance ORDER BY period_start"))
    {
        showError("Ошибка запроса балансов: " + query.lastError().text());
        return;
    }

    while (query.next())
    {
        int balanceId = query.value("id").toInt();
        QDate createDate = query.value("create_date").toDateTime().date();
        QDate periodStart = query.value("period_start").toDate();
        QDate periodEnd = query.value("period_end").toDate();
        double debit = query.value("debit").toDouble();
        double credit = query.value("credit").toDouble();
        double amount = query.value("amount").toDouble();

        html += "<div class='balance'>";
        html += QString("<h2>Баланс ID: %1 — период %2 — %3</h2>")
                .arg(balanceId)
                .arg(periodStart.toString("dd.MM.yyyy"))
                .arg(periodEnd.toString("dd.MM.yyyy"));

        html += QString("<p>Дата формирования: %1</p>").arg(createDate.toString("dd.MM.yyyy"));
        html += QString("<p>Приход: %1 ₽ &nbsp;&nbsp; Расход: %2 ₽ &nbsp;&nbsp; Прибыль: %3 ₽</p>")
                .arg(debit, 0, 'f', 2)
                .arg(credit, 0, 'f', 2)
                .arg(amount, 0, 'f', 2);

        // Таблица операций для этого баланса
        QSqlQuery opq(db);
        opq.prepare(R"(
            SELECT o.id, a.name AS article_name, o.debit, o.credit, o.create_date
            FROM operations o
            JOIN articles a ON o.article_id = a.id
            WHERE o.balance_id = :balId
            ORDER BY o.create_date
        )");
        opq.bindValue(":balId", balanceId);
        if (!opq.exec())
        {
            html += "<p><i>Ошибка при получении операций: " + opq.lastError().text() + "</i></p>";
        }
        else
        {
            html += "<table>";
            html += "<tr><th>ID</th><th>Статья</th><th>Приход</th><th>Расход</th><th>Дата</th></tr>";
            while (opq.next())
            {
                int oid = opq.value("id").toInt();
                QString article = opq.value("article_name").toString();
                double od = opq.value("debit").toDouble();
                double oc = opq.value("credit").toDouble();
                QDateTime odate = opq.value("create_date").toDateTime();

                html += QString("<tr>"
                                "<td>%1</td>"
                                "<td>%2</td>"
                                "<td align='right'>%3</td>"
                                "<td align='right'>%4</td>"
                                "<td>%5</td>"
                                "</tr>")
                        .arg(oid)
                        .arg(article.toHtmlEscaped())
                        .arg(od, 0, 'f', 2)
                        .arg(oc, 0, 'f', 2)
                        .arg(odate.toString("dd.MM.yyyy hh:mm:ss"));
            }
            html += "</table>";
        }

        html += "</div>"; // balance
    }

    html += "</body></html>";

    // Печатаем HTML в PDF
    QTextDocument doc;
    doc.setHtml(html);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    printer.setPageMargins(QMarginsF(10, 10, 10, 10));

    doc.print(&printer);

    QMessageBox::information(this, "Экспорт", "PDF-отчёт успешно создан:\n" + filename);
}

void MainWindow::on_actionExportCSV_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Excel (CSV) Report", QString(), "CSV Files (*.csv);;Excel Files (*.xls *.xlsx)");
    if (filename.isEmpty()) return;

    if (!filename.endsWith(".csv", Qt::CaseInsensitive))
    {
        if (filename.endsWith(".xls", Qt::CaseInsensitive) || filename.endsWith(".xlsx", Qt::CaseInsensitive))
            filename = QFileInfo(filename).path() + "/" + QFileInfo(filename).completeBaseName() + ".csv";
        else
            filename += ".csv";
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        showError("Не удалось создать файл: " + filename);
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    const QByteArray bom = QByteArray::fromHex("EFBBBF");
    file.write(bom);

    QSqlDatabase db = databaseManager->getDatabase();
    QSqlQuery query(db);

    // Заголовки: баланс (как строка), затем операции
    if (!query.exec("SELECT id, create_date, period_start, period_end, debit, credit, amount FROM balance ORDER BY period_start"))
    {
        showError("Ошибка запроса балансов: " + query.lastError().text());
        file.close();
        return;
    }

    while (query.next())
    {
        int balanceId = query.value("id").toInt();
        QDate createDate = query.value("create_date").toDateTime().date();
        QDate periodStart = query.value("period_start").toDate();
        QDate periodEnd = query.value("period_end").toDate();
        double debit = query.value("debit").toDouble();
        double credit = query.value("credit").toDouble();
        double amount = query.value("amount").toDouble();

        out << "Баланс ID:," << balanceId << "\n";
        out << "Период:," << periodStart.toString("dd.MM.yyyy") << " - " << periodEnd.toString("dd.MM.yyyy") << "\n";
        out << "Дата формирования:," << createDate.toString("dd.MM.yyyy") << "\n";
        out << "Приход:, " << QString::number(debit, 'f', 2) << ", Расход:, " << QString::number(credit, 'f', 2) << ", Прибыль:, " << QString::number(amount, 'f', 2) << "\n";

        // таблица операций
        QSqlQuery opq(db);
        opq.prepare(R"(
            SELECT o.id, a.name AS article_name, o.debit, o.credit, o.create_date
            FROM operations o
            JOIN articles a ON o.article_id = a.id
            WHERE o.balance_id = :balId
            ORDER BY o.create_date
        )");
        opq.bindValue(":balId", balanceId);
        if (!opq.exec())
        {
            out << "Ошибка получения операций:," << opq.lastError().text() << "\n\n";
            continue;
        }

        out << "ID,Статья,Приход,Расход,Дата\n";
        while (opq.next())
        {
            int oid = opq.value("id").toInt();
            QString article = opq.value("article_name").toString();
            double od = opq.value("debit").toDouble();
            double oc = opq.value("credit").toDouble();
            QDateTime odate = opq.value("create_date").toDateTime();

            // Экранируем запятые, оборачиваем в кавычки
            auto esc = [](const QString &s){ QString r = s; r.replace('"', "\"\""); return QString("\"%1\"").arg(r); };
            out << oid << "," << esc(article) << "," << QString::number(od, 'f', 2) << "," << QString::number(oc, 'f', 2) << "," << esc(odate.toString("dd.MM.yyyy hh:mm:ss")) << "\n";
        }
        out << "\n\n";
    }

    file.close();
    QMessageBox::information(this, "Экспорт", "CSV-отчёт успешно создан:\n" + filename);
}
