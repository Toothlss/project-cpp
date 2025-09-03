#ifndef COMMANDEWIDGET_H
#define COMMANDEWIDGET_H
#include <QWidget>
#include <QSqlTableModel>

namespace Ui
{
    class CommandeWidget;
}

class CommandeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CommandeWidget(QWidget *parent = nullptr);
    ~CommandeWidget();

private slots:
    void addCommande();
    void editCommande();
    void deleteCommande();
    void searchCommandes();
    void sortCommandes(int index);
    void showStats();
    void exportPDF();

private:
    Ui::CommandeWidget *ui;
    QSqlTableModel *model;
    void setupModel();
    void setupConnections();
};

#endif // COMMANDEWIDGET_H
