#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H
#include <QWidget>
#include <QSqlTableModel>

namespace Ui
{
    class ClientWidget;
}

class ClientWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClientWidget(QWidget *parent = nullptr);
    ~ClientWidget();

private slots:
    void addClient();
    void editClient();
    void deleteClient();

private:
    Ui::ClientWidget *ui;
    QSqlTableModel *model;
    void setupModel();
    void setupConnections();
};

#endif // CLIENTWIDGET_H
