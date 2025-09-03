#ifndef COMMANDE_H
#define COMMANDE_H
#include <QString>
#include <QDate>

class Commande
{
public:
    Commande();
    Commande(int id, const QString &client, const QString &etat, const QDate &date, double montant);

    int getId() const;
    QString getClient() const;
    QString getEtat() const;
    QDate getDate() const;
    double getMontant() const;

    void setClient(const QString &client);
    void setEtat(const QString &etat);
    void setDate(const QDate &date);
    void setMontant(double montant);

private:
    int id;
    QString client;
    QString etat;
    QDate date;
    double montant;
};

#endif // COMMANDE_H
