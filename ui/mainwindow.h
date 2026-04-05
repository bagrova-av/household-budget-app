#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QDate>

#include "../app/databasemanager.h"
#include "../app/analyticsmanager.h"
#include "../models/articlemodel.h"
#include "../models/operationmodel.h"
#include "../models/balancemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    QString userRole;

    DatabaseManager *databaseManager;
    AnalyticsManager *analyticsManager;

    ArticleModel *articleModel;
    OperationModel *operationModel;
    BalanceModel *balanceModel;

    void showError(const QString& message);
    void showInfo(const QString& message);

    void generateDynamicsChart(const QDateTime& startDate, const QDateTime& endDate,
                               const QVector<int>& articleIds);
    void generateDynamicsBarChart(const QDateTime& startDate, const QDateTime& endDate,
                                         const QVector<int>& articleIds);
    void generatePieChart(const QDateTime& startDate, const QDateTime& endDate,
                          const QVector<int>& articleIds,
                          const QString& flowType);
    void generateProfitChart(const QDateTime& startDate, const QDateTime& endDate);

    void refreshArticlesList();

private slots:
    void on_addOperationButton_clicked();
    void on_editOperationButton_clicked();
    void on_deleteOperationButton_clicked();

    void on_createBalanceButton_clicked();
    void on_disbandBalanceButton_clicked();

    void on_addArticleButton_clicked();
    void on_deleteArticleButton_clicked();

    void on_updateChartsButton_clicked();

    void on_actionExportPDF_triggered();
    void on_actionExportCSV_triggered();

public:
    explicit MainWindow(DatabaseManager* dbManager, const QString& currentUserRole, QWidget *parent = nullptr);
    ~MainWindow();
};
#endif // MAINWINDOW_H
