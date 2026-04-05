#include "operationdialog.h"
#include "./ui_operationdialog.h"

AddOperationDialog::AddOperationDialog(const QMap<int, QString>& articles, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddOperationDialog),
    articles(articles)
{
    ui->setupUi(this);
    setWindowTitle("Добавить операцию");

    // Заполняем комбобокс статьями
    for (auto it = articles.begin(); it != articles.end(); ++it)
    {
        ui->articleComboBox->addItem(it.value(), it.key());
    }

    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    ui->debitSpinBox->setMinimum(0);
    ui->debitSpinBox->setMaximum(1000000);
    ui->debitSpinBox->setDecimals(2);

    ui->creditSpinBox->setMinimum(0);
    ui->creditSpinBox->setMaximum(1000000);
    ui->creditSpinBox->setDecimals(2);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void AddOperationDialog::setInitialValues(const QString& currentArticleName,
                                          double currentDebit,double currentCredit,
                                          const QDateTime& currentDate)
{
    setWindowTitle("Редактировать операцию");

    ui->articleComboBox->setCurrentText(currentArticleName);
    ui->debitSpinBox->setValue(currentDebit);
    ui->creditSpinBox->setValue(currentCredit);
    ui->dateTimeEdit->setDateTime(currentDate);
}

QString AddOperationDialog::getArticleName() const
{
    return ui->articleComboBox->currentText();
}

double AddOperationDialog::getDebit() const
{
    return ui->debitSpinBox->value();
}

double AddOperationDialog::getCredit() const
{
    return ui->creditSpinBox->value();
}

QDateTime AddOperationDialog::getOperationDate() const
{
    return ui->dateTimeEdit->dateTime();
}

AddOperationDialog::~AddOperationDialog()
{
    delete ui;
}

