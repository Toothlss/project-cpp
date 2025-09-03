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
#include <QtWidgets/QFileDialog>
#include <QtGui/QPdfWriter>
#include <QtGui/QPainter>
#include <QtGui/QPageSize>
#include <QLocale>
#include <QFontMetrics>

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
        QMessageBox::warning(this, tr("Erreur"), tr("Impossible d'ouvrir le fichier PDF pour l'écriture."));
        return;
    }

    const int margin = 100; // device units (~0.33in at 300dpi)
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
    QString title = tr("Liste des commandes");
    painter.drawText(margin, y, contentW, 60, Qt::AlignCenter, title);
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
    int cClient = model->fieldIndex("client");
    int cEtat = model->fieldIndex("etat");
    int cDate = model->fieldIndex("date");
    int cMontant = model->fieldIndex("montant");

    if (cId >= 0 || cClient >= 0 || cEtat >= 0 || cDate >= 0 || cMontant >= 0)
    {
        addIfValid(cId, tr("ID"), Qt::AlignCenter, 0.10, 80);
        addIfValid(cClient, tr("Client"), Qt::AlignLeft | Qt::AlignVCenter, 0.30, 150);
        addIfValid(cEtat, tr("État"), Qt::AlignLeft | Qt::AlignVCenter, 0.18, 100);
        addIfValid(cDate, tr("Date"), Qt::AlignCenter, 0.18, 110);
        addIfValid(cMontant, tr("Montant"), Qt::AlignRight | Qt::AlignVCenter, 0.24, 120);
    }
    else
    {
        // Fallback: use headers from model
        for (int col = 0; col < model->columnCount(); ++col)
        {
            QString hdr = model->headerData(col, Qt::Horizontal).toString();
            if (!hdr.isEmpty())
                hdr[0] = hdr[0].toUpper();
            cols.append(Col{col, hdr, Qt::AlignLeft | Qt::AlignVCenter, 1.0 / qMax(1, model->columnCount()), 100});
        }
    }

    // Normalize weights and compute column widths
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
    // If total wider than content, scale down proportionally but keep mins
    int totalW = 0;
    for (int w : colWidths)
        totalW += w;
    if (totalW > contentW)
    {
        double scale = double(contentW) / double(totalW);
        for (int i = 0; i < colWidths.size(); ++i)
        {
            colWidths[i] = qMax(int(colWidths[i] * scale), cols[i].min);
        }
        // Recompute total and clamp last column to fit exactly
        totalW = 0;
        for (int w : colWidths)
            totalW += w;
        if (totalW > contentW)
            colWidths.last() -= (totalW - contentW);
    }

    // Header drawing lambda (repeated on each page)
    auto drawHeader = [&](int yHeader) -> int
    {
        QFont headerFont = painter.font();
        headerFont.setBold(true);
        painter.setFont(headerFont);
        QFontMetrics fm(headerFont);
        int headerHeight = fm.height() + 2 * cellVPad;

        int x = margin;
        // background
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
        painter.setFont(QFont()); // reset to default (non-bold)
        return headerHeight;
    };

    // Footer helper: page number
    auto drawFooter = [&](int pageNumber)
    {
        QString footer = tr("Page %1").arg(pageNumber);
        painter.drawText(margin, pageH - margin + 20, contentW, 40, Qt::AlignRight | Qt::AlignVCenter, footer);
    };

    int pageNumber = 1;
    int headerHeight = drawHeader(y);
    y += headerHeight;

    // Body rows
    QFont bodyFont = painter.font();
    painter.setFont(bodyFont);
    QFontMetrics fm(bodyFont);
    QLocale locale = QLocale::system();

    for (int row = 0; row < model->rowCount(); ++row)
    {
        // Compute row height based on wrapped text
        int rowHeight = 0;
        for (int i = 0; i < cols.size(); ++i)
        {
            QString text;
            QVariant v = model->data(model->index(row, cols[i].idx));
            if (cols[i].header.compare(tr("Montant")) == 0)
                text = locale.toString(v.toDouble(), 'f', 2);
            else
                text = v.toString();

            QRect br = fm.boundingRect(0, 0, colWidths[i] - 2 * cellHPad, 10000,
                                       Qt::AlignLeft | Qt::TextWordWrap, text);
            rowHeight = qMax(rowHeight, br.height() + 2 * cellVPad);
        }
        // Page break if needed
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

            // Alignment and formatting
            Qt::Alignment align = cols[i].align;
            QString text;
            QVariant v = model->data(model->index(row, cols[i].idx));
            if (cols[i].header.compare(tr("Date")) == 0)
            {
                QDate d = QDate::fromString(v.toString(), "yyyy-MM-dd");
                text = d.isValid() ? d.toString("yyyy-MM-dd") : v.toString();
                align = Qt::AlignCenter;
            }
            else if (cols[i].header.compare(tr("Montant")) == 0)
            {
                text = locale.toString(v.toDouble(), 'f', 2);
                align = Qt::AlignRight | Qt::AlignVCenter;
            }
            else if (cols[i].header.compare(tr("ID")) == 0)
            {
                text = v.toString();
                align = Qt::AlignCenter;
            }
            else
            {
                text = v.toString();
            }

            painter.drawText(rect.adjusted(cellHPad, 0, -cellHPad, 0),
                             align | Qt::TextWordWrap, text);
            x += colWidths[i];
        }
        y += rowHeight;
    }

    drawFooter(pageNumber);
    painter.end();
    QMessageBox::information(this, tr("Export PDF"), tr("Export PDF terminé avec succès !"));
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
