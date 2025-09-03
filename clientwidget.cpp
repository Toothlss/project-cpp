#include "clientwidget.h"
#include "ui_client.h"

#include <QMessageBox>
#include <QSqlRecord>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QInputDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

ClientWidget::ClientWidget(QWidget *parent) : QWidget(parent),
                                              ui(new Ui::ClientWidget),
                                              model(new QSqlTableModel(this))
{
    ui->setupUi(this);
    setupModel();
    setupConnections();
}

ClientWidget::~ClientWidget()
{
    delete ui;
}

// --------------------
// Setup
// --------------------
void ClientWidget::setupModel()
{
    model->setTable("client");
    model->select();
    ui->tableClients->setModel(model);
}

void ClientWidget::setupConnections()
{
    connect(ui->btnAddClient, &QPushButton::clicked, this, &ClientWidget::addClient);
    connect(ui->btnEditClient, &QPushButton::clicked, this, &ClientWidget::editClient);
    connect(ui->btnDeleteClient, &QPushButton::clicked, this, &ClientWidget::deleteClient);
    connect(ui->btnSearch, &QPushButton::clicked, this, &ClientWidget::searchClients);
    connect(ui->sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClientWidget::sortClients);
    connect(ui->btnStats, &QPushButton::clicked, this, &ClientWidget::showStats);
    connect(ui->btnExportPDF, &QPushButton::clicked, this, &ClientWidget::exportPDF);
}

// --------------------
// CRUD Slots
// --------------------
void ClientWidget::addClient()
{
    bool ok;
    QString nom = QInputDialog::getText(this, "Nom", "Nom:", QLineEdit::Normal, "", &ok);
    if (!ok || nom.isEmpty())
        return;
    QString prenom = QInputDialog::getText(this, "Prénom", "Prénom:", QLineEdit::Normal, "", &ok);
    if (!ok || prenom.isEmpty())
        return;
    QString adresse = QInputDialog::getText(this, "Adresse", "Adresse:", QLineEdit::Normal, "", &ok);
    if (!ok || adresse.isEmpty())
        return;
    QString telephone = QInputDialog::getText(this, "Téléphone", "Téléphone:", QLineEdit::Normal, "", &ok);
    if (!ok || telephone.isEmpty())
        return;

    QSqlRecord record = model->record();
    record.setValue("nom", nom);
    record.setValue("prenom", prenom);
    record.setValue("adresse", adresse);
    record.setValue("telephone", telephone);
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

void ClientWidget::editClient()
{
    QModelIndexList selection = ui->tableClients->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;
    int row = selection.first().row();
    QSqlRecord record = model->record(row);
    bool ok;
    QString nom = QInputDialog::getText(this, "Nom", "Nom:", QLineEdit::Normal, record.value("nom").toString(), &ok);
    if (!ok || nom.isEmpty())
        return;
    QString prenom = QInputDialog::getText(this, "Prénom", "Prénom:", QLineEdit::Normal, record.value("prenom").toString(), &ok);
    if (!ok || prenom.isEmpty())
        return;
    QString adresse = QInputDialog::getText(this, "Adresse", "Adresse:", QLineEdit::Normal, record.value("adresse").toString(), &ok);
    if (!ok || adresse.isEmpty())
        return;
    QString telephone = QInputDialog::getText(this, "Téléphone", "Téléphone:", QLineEdit::Normal, record.value("telephone").toString(), &ok);
    if (!ok || telephone.isEmpty())
        return;

    record.setValue("nom", nom);
    record.setValue("prenom", prenom);
    record.setValue("adresse", adresse);
    record.setValue("telephone", telephone);
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

void ClientWidget::deleteClient()
{
    QModelIndex index = ui->tableClients->currentIndex();
    if (!index.isValid())
    {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un client à supprimer !");
        return;
    }

    model->removeRow(index.row());
    if (!model->submitAll())
    {
        QMessageBox::warning(this, "Erreur", "Impossible de supprimer le client !");
    }
    model->select();
}

// --------------------
// Search & Sort
// --------------------
void ClientWidget::searchClients()
{
    QStringList filters;

    if (!ui->searchNom->text().isEmpty())
        filters << QString("nom LIKE '%%1%'").arg(ui->searchNom->text());
    if (!ui->searchPrenom->text().isEmpty())
        filters << QString("prenom LIKE '%%1%'").arg(ui->searchPrenom->text());
    if (!ui->searchAdresse->text().isEmpty())
        filters << QString("adresse LIKE '%%1%'").arg(ui->searchAdresse->text());

    model->setFilter(filters.join(" AND "));
    model->select();
}

void ClientWidget::sortClients(int index)
{
    switch (index)
    {
    case 1: // Nom
        model->setSort(model->fieldIndex("nom"), Qt::AscendingOrder);
        break;
    case 2: // Prénom
        model->setSort(model->fieldIndex("prenom"), Qt::AscendingOrder);
        break;
    case 3: // Adresse
        model->setSort(model->fieldIndex("adresse"), Qt::AscendingOrder);
        break;
    default:
        model->setSort(-1, Qt::AscendingOrder);
        break;
    }
    model->select();
}

// --------------------
// Extra Features
// --------------------
void ClientWidget::showStats()
{
    // Count clients by city (adresse)
    QMap<QString, int> cityCount;
    for (int row = 0; row < model->rowCount(); ++row)
    {
        QString city = model->data(model->index(row, model->fieldIndex("adresse"))).toString();
        cityCount[city]++;
    }

    QBarSet *set = new QBarSet("Clients");
    QStringList categories;
    for (auto it = cityCount.begin(); it != cityCount.end(); ++it)
    {
        *set << it.value();
        categories << it.key();
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Nombre de clients par ville");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    ui->clientStatsChart->setChart(chart);
    ui->clientStatsChart->setRenderHint(QPainter::Antialiasing);
}

void ClientWidget::exportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty())
        return;

    QPdfWriter writer(fileName);
    QPainter painter(&writer);

    painter.drawText(100, 100, "Export des clients - Exemple PDF");
    // TODO: Add table export here
    painter.end();

    QMessageBox::information(this, "Export PDF", "Export PDF terminé avec succès !");
}
