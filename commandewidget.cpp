#include "commandewidget.h"
#include "ui_commande.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QAbstractItemView>
#include <QLineEdit>

CommandeWidget::CommandeWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CommandeWidget), model(nullptr)
{
    ui->setupUi(this);
    ui->searchDate->setDate(QDate::currentDate());
    setupModel();
    setupConnections();
}

CommandeWidget::~CommandeWidget()
{
    delete model;
    delete ui;
}

void CommandeWidget::setupModel()
{
    model = new QSqlTableModel(this);
    model->setTable("commande");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    ui->tableCommandes->setModel(model);
    ui->tableCommandes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableCommandes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableCommandes->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void CommandeWidget::setupConnections()
{
    connect(ui->btnAddCommande, &QPushButton::clicked, this, &CommandeWidget::addCommande);
    connect(ui->btnEditCommande, &QPushButton::clicked, this, &CommandeWidget::editCommande);
    connect(ui->btnDeleteCommande, &QPushButton::clicked, this, &CommandeWidget::deleteCommande);
    connect(ui->btnSearch, &QPushButton::clicked, this, &CommandeWidget::searchCommandes);
    connect(ui->sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CommandeWidget::sortCommandes);
    connect(ui->btnStats, &QPushButton::clicked, this, &CommandeWidget::showStats);
    connect(ui->btnExportPDF, &QPushButton::clicked, this, &CommandeWidget::exportPDF);
}

void CommandeWidget::searchCommandes()
{
    QString filter;
    QString client = ui->searchClient->text();
    QString etat = ui->searchEtat->currentText();
    QDate date = ui->searchDate->date();
    QStringList filters;

    if (!client.isEmpty())
        filters << QString("client LIKE '%%" + client + "%%'");
    if (etat != "Tout état")
        filters << QString("etat = '%1'").arg(etat);
    // Only filter by date if the user has changed it from the default
    QDate defaultDate = QDate::currentDate();
    if (date != defaultDate)
        filters << QString("date = '%1'").arg(date.toString("yyyy-MM-dd"));

    filter = filters.join(" AND ");
    model->setFilter(filter);
    model->select();
}

void CommandeWidget::sortCommandes(int index)
{
    switch (index)
    {
    case 1: // Client
        model->setSort(model->fieldIndex("client"), Qt::AscendingOrder);
        break;
    case 2: // Date
        model->setSort(model->fieldIndex("date"), Qt::AscendingOrder);
        break;
    case 3: // Montant
        model->setSort(model->fieldIndex("montant"), Qt::DescendingOrder);
        break;
    default:
        model->setSort(-1, Qt::AscendingOrder);
    }
    model->select();
}

void CommandeWidget::showStats()
{
    // Count commandes by status
    QMap<QString, int> statusCount;
    for (int row = 0; row < model->rowCount(); ++row)
    {
        QString etat = model->data(model->index(row, model->fieldIndex("etat"))).toString();
        statusCount[etat]++;
    }

    QBarSet *set = new QBarSet("Commandes");
    QStringList categories;
    for (auto it = statusCount.begin(); it != statusCount.end(); ++it)
    {
        *set << it.value();
        categories << it.key();
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Nombre de commandes par état");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    ui->commandeStatsChart->setChart(chart);
    ui->commandeStatsChart->setRenderHint(QPainter::Antialiasing);
}

void CommandeWidget::exportPDF()
{
    QMessageBox::information(this, "Export PDF", "Export PDF à implémenter.");
}

void CommandeWidget::addCommande()
{
    bool ok;
    QString client = QInputDialog::getText(this, "Client", "Nom du client:", QLineEdit::Normal, "", &ok);
    if (!ok || client.isEmpty())
        return;

    QStringList etatOptions;
    etatOptions << "En attente" << "Livrée" << "Annulée";
    QString etat = QInputDialog::getItem(this, "Etat", "Etat:", etatOptions, 0, false, &ok);
    if (!ok || etat.isEmpty())
        return;

    QString dateStr = QInputDialog::getText(this, "Date", "Date (YYYY-MM-DD):",
                                            QLineEdit::Normal, QDate::currentDate().toString("yyyy-MM-dd"), &ok);
    if (!ok || dateStr.isEmpty())
        return;

    double montant = QInputDialog::getDouble(this, "Montant", "Montant:", 0, 0, 1000000, 2, &ok);
    if (!ok)
        return;

    QSqlRecord record = model->record();
    record.setValue("client", client);
    record.setValue("etat", etat);
    record.setValue("date", dateStr);
    record.setValue("montant", montant);

    if (!model->insertRecord(-1, record) || !model->submitAll())
    {
        QMessageBox::warning(this, "Erreur", "Ajout échoué.");
        model->revertAll();
    }
    else
    {
        model->select();
    }
}

void CommandeWidget::editCommande()
{
    QModelIndexList selection = ui->tableCommandes->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;

    int row = selection.first().row();
    QSqlRecord record = model->record(row);

    bool ok;
    QString client = QInputDialog::getText(this, "Client", "Nom du client:", QLineEdit::Normal, record.value("client").toString(), &ok);
    if (!ok || client.isEmpty())
        return;

    QStringList etatOptions;
    etatOptions << "En attente" << "Livrée" << "Annulée";
    QString etat = QInputDialog::getItem(this, "Etat", "Etat:", etatOptions, etatOptions.indexOf(record.value("etat").toString()), false, &ok);
    if (!ok || etat.isEmpty())
        return;

    QString dateStr = QInputDialog::getText(this, "Date", "Date (YYYY-MM-DD):", QLineEdit::Normal, record.value("date").toString(), &ok);
    if (!ok || dateStr.isEmpty())
        return;

    double montant = QInputDialog::getDouble(this, "Montant", "Montant:", record.value("montant").toDouble(), 0, 1000000, 2, &ok);
    if (!ok)
        return;

    record.setValue("client", client);
    record.setValue("etat", etat);
    record.setValue("date", dateStr);
    record.setValue("montant", montant);

    if (!model->setRecord(row, record) || !model->submitAll())
    {
        QMessageBox::warning(this, "Erreur", "Modification échouée.");
        model->revertAll();
    }
    else
    {
        model->select();
    }
}

void CommandeWidget::deleteCommande()
{
    QModelIndexList selection = ui->tableCommandes->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;

    int row = selection.first().row();
    if (QMessageBox::question(this, "Confirmation", "Supprimer cette commande ?") == QMessageBox::Yes)
    {
        model->removeRow(row);
        if (!model->submitAll())
        {
            QMessageBox::warning(this, "Erreur", "Suppression échouée.");
            model->revertAll();
        }
        else
        {
            model->select();
        }
    }
}
