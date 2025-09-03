#include "commandewidget.h"
#include "ui_commande.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>

CommandeWidget::CommandeWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CommandeWidget), model(nullptr)
{
    ui->setupUi(this);
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
}

void CommandeWidget::addCommande()
{
    // Simple dialog-based input for demonstration
    bool ok;
    QString client = QInputDialog::getText(this, "Client", "Nom du client:", QLineEdit::Normal, "", &ok);
    if (!ok || client.isEmpty())
        return;
    QString etat = QInputDialog::getText(this, "Etat", "Etat:", QLineEdit::Normal, "", &ok);
    if (!ok || etat.isEmpty())
        return;
    QString dateStr = QInputDialog::getText(this, "Date", "Date (YYYY-MM-DD):", QLineEdit::Normal, QDate::currentDate().toString("yyyy-MM-dd"), &ok);
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
    QString etat = QInputDialog::getText(this, "Etat", "Etat:", QLineEdit::Normal, record.value("etat").toString(), &ok);
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
