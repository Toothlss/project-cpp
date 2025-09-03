#include "commande.h"

Commande::Commande() : id(0), montant(0.0) {}

Commande::Commande(int id, const QString &client, const QString &etat, const QDate &date, double montant)
    : id(id), client(client), etat(etat), date(date), montant(montant) {}

int Commande::getId() const { return id; }
QString Commande::getClient() const { return client; }
QString Commande::getEtat() const { return etat; }
QDate Commande::getDate() const { return date; }
double Commande::getMontant() const { return montant; }

void Commande::setClient(const QString &client) { this->client = client; }
void Commande::setEtat(const QString &etat) { this->etat = etat; }
void Commande::setDate(const QDate &date) { this->date = date; }
void Commande::setMontant(double montant) { this->montant = montant; }
