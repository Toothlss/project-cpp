#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class CommandeWidget;
class ClientWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showCommandeWidget();
    void showClientWidget();

private:
    Ui::MainWindow *ui;
    CommandeWidget *commandeWidget;
    ClientWidget *clientWidget;
};

#endif // MAINWINDOW_H
