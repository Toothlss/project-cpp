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
#include <QLocale>
#include <QFontMetrics>

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
    QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en PDF"), "", tr("Fichiers PDF (*.pdf)"));
    if (fileName.isEmpty())
        return;
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
        fileName += ".pdf";

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);

    QPainter painter(&writer);
    if (!painter.isActive())
    {
        QMessageBox::warning(this, tr("Erreur"), tr("Impossible de créer le PDF."));
        return;
    }

    const int margin = 100; // ~0.33in at 300dpi
    const int pageW = writer.width();
    const int pageH = writer.height();
    const int contentW = pageW - 2 * margin;
    const int cellHPad = 12;
    const int cellVPad = 10;

    // Title
    int y = margin;
    QFont titleFont = painter.font();
    titleFont.setPointSize(titleFont.pointSize() + 4);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(margin, y, contentW, 60, Qt::AlignCenter, tr("Liste des clients"));
    y += 70;

    // Prepare columns (prefer known order; fallback to table order)
    struct Col
    {
        int idx;
        QString header;
        Qt::Alignment align;
        double weight;
        int min;
    };
    QList<Col> cols;
    auto addIfValid = [&](int idx, const QString &header, Qt::Alignment align, double weight, int min)
    {
        if (idx >= 0)
            cols.append(Col{idx, header, align, weight, min});
    };

    int cId = model->fieldIndex("id");
    int cNom = model->fieldIndex("nom");
    int cPrenom = model->fieldIndex("prenom");
    int cAdresse = model->fieldIndex("adresse");
    int cTel = model->fieldIndex("telephone");

    if (cId >= 0 || cNom >= 0 || cPrenom >= 0 || cAdresse >= 0 || cTel >= 0)
    {
        addIfValid(cId, tr("ID"), Qt::AlignCenter, 0.10, 80);
        addIfValid(cNom, tr("Nom"), Qt::AlignLeft | Qt::AlignVCenter, 0.22, 120);
        addIfValid(cPrenom, tr("Prénom"), Qt::AlignLeft | Qt::AlignVCenter, 0.22, 120);
        addIfValid(cAdresse, tr("Adresse"), Qt::AlignLeft | Qt::AlignVCenter, 0.28, 160);
        addIfValid(cTel, tr("Téléphone"), Qt::AlignCenter, 0.18, 120);
    }
    else
    {
        // Fallback to whatever headers the model exposes
        for (int col = 0; col < model->columnCount(); ++col)
        {
            QString hdr = model->headerData(col, Qt::Horizontal).toString();
            if (!hdr.isEmpty())
                hdr[0] = hdr[0].toUpper();
            cols.append(Col{col, hdr, Qt::AlignLeft | Qt::AlignVCenter, 1.0 / qMax(1, model->columnCount()), 100});
        }
    }

    // Normalize weights and compute widths
    double weightSum = 0.0;
    for (const Col &c : cols)
        weightSum += c.weight;
    if (weightSum <= 0.0)
        weightSum = 1.0;

    QVector<int> colWidths;
    colWidths.reserve(cols.size());
    for (const Col &c : cols)
    {
        int w = qMax(int((c.weight / weightSum) * contentW), c.min);
        colWidths.append(w);
    }
    int totalW = 0;
    for (int w : colWidths)
        totalW += w;
    if (totalW > contentW)
    {
        double scale = double(contentW) / double(totalW);
        for (int i = 0; i < colWidths.size(); ++i)
            colWidths[i] = qMax(int(colWidths[i] * scale), cols[i].min);
        totalW = 0;
        for (int w : colWidths)
            totalW += w;
        if (totalW > contentW)
            colWidths.last() -= (totalW - contentW);
    }

    // Header drawer (re-used on each page)
    auto drawHeader = [&](int yHeader) -> int
    {
        QFont headerFont = painter.font();
        headerFont.setBold(true);
        painter.setFont(headerFont);
        QFontMetrics fm(headerFont);
        int headerHeight = fm.height() + 2 * cellVPad;

        int x = margin;
        painter.save();
        painter.setBrush(QColor(240, 240, 240));
        painter.setPen(Qt::NoPen);
        painter.drawRect(margin, yHeader, totalW, headerHeight);
        painter.restore();

        painter.setPen(Qt::black);
        for (int i = 0; i < cols.size(); ++i)
        {
            QRect rect(x, yHeader, colWidths[i], headerHeight);
            painter.drawRect(rect);
            painter.drawText(rect.adjusted(cellHPad, 0, -cellHPad, 0), Qt::AlignVCenter | Qt::AlignLeft, cols[i].header);
            x += colWidths[i];
        }
        painter.setFont(QFont()); // reset to normal
        return headerHeight;
    };

    // Footer with page number
    auto drawFooter = [&](int pageNumber)
    {
        painter.drawText(margin, pageH - margin + 20, contentW, 40,
                         Qt::AlignRight | Qt::AlignVCenter, tr("Page %1").arg(pageNumber));
    };

    int pageNumber = 1;
    int headerHeight = drawHeader(y);
    y += headerHeight;

    // Body
    QFont bodyFont = painter.font();
    painter.setFont(bodyFont);
    QFontMetrics fm(bodyFont);

    for (int row = 0; row < model->rowCount(); ++row)
    {
        // Determine row height based on wrapped content
        int rowHeight = 0;
        for (int i = 0; i < cols.size(); ++i)
        {
            QString text = model->data(model->index(row, cols[i].idx)).toString();
            QRect br = fm.boundingRect(0, 0, colWidths[i] - 2 * cellHPad, 10000,
                                       Qt::AlignLeft | Qt::TextWordWrap, text);
            rowHeight = qMax(rowHeight, br.height() + 2 * cellVPad);
        }

        // Page break
        if (y + rowHeight > pageH - margin)
        {
            drawFooter(pageNumber);
            writer.newPage();
            pageNumber++;
            y = margin;
            headerHeight = drawHeader(y);
            y += headerHeight;
        }

        // Draw row cells
        int x = margin;
        for (int i = 0; i < cols.size(); ++i)
        {
            QRect rect(x, y, colWidths[i], rowHeight);
            painter.drawRect(rect);

            QString text = model->data(model->index(row, cols[i].idx)).toString();
            Qt::Alignment align = cols[i].align;

            // Specific alignment tweaks
            if (cols[i].header.compare(tr("ID")) == 0)
                align = Qt::AlignCenter;
            else if (cols[i].header.compare(tr("Téléphone")) == 0)
                align = Qt::AlignCenter;

            painter.drawText(rect.adjusted(cellHPad, 0, -cellHPad, 0),
                             align | Qt::TextWordWrap, text);
            x += colWidths[i];
        }
        y += rowHeight;
    }

    drawFooter(pageNumber);
    painter.end();
    QMessageBox::information(this, tr("Succès"), tr("PDF exporté : %1").arg(fileName));
}