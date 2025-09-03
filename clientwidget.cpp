#include "clientwidget.h"
#include "ui_client.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QInputDialog>

ClientWidget::ClientWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ClientWidget), model(nullptr)
{
    ui->setupUi(this);
    setupModel();
    setupConnections();
}

ClientWidget::~ClientWidget()
{
    delete model;
    delete ui;
}

void ClientWidget::setupModel()
{
    model = new QSqlTableModel(this);
    model->setTable("client");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    ui->tableClients->setModel(model);
    ui->tableClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableClients->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableClients->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ClientWidget::setupConnections()
{
    connect(ui->btnAddClient, &QPushButton::clicked, this, &ClientWidget::addClient);
    connect(ui->btnEditClient, &QPushButton::clicked, this, &ClientWidget::editClient);
    connect(ui->btnDeleteClient, &QPushButton::clicked, this, &ClientWidget::deleteClient);
}

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
    QModelIndexList selection = ui->tableClients->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;
    int row = selection.first().row();
    if (QMessageBox::question(this, "Confirmation", "Supprimer ce client ?") == QMessageBox::Yes)
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
