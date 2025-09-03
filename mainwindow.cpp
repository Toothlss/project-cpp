
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commandewidget.h"
#include "clientwidget.h"

#include "database.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), commandeWidget(nullptr), clientWidget(nullptr)
{
    ui->setupUi(this);
    Database::initialize();
    connect(ui->btnCommande, &QPushButton::clicked, this, &MainWindow::showCommandeWidget);
    connect(ui->btnClient, &QPushButton::clicked, this, &MainWindow::showClientWidget);
}

MainWindow::~MainWindow()
{
    delete commandeWidget;
    delete clientWidget;
    delete ui;
}

void MainWindow::showCommandeWidget()
{
    if (!commandeWidget)
        commandeWidget = new CommandeWidget(this);
    commandeWidget->show();
    commandeWidget->raise();
    commandeWidget->activateWindow();
}

void MainWindow::showClientWidget()
{
    if (!clientWidget)
        clientWidget = new ClientWidget(this);
    clientWidget->show();
    clientWidget->raise();
    clientWidget->activateWindow();
}
