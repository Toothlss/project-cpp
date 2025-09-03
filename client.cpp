#include "client.h"

Client::Client() : id(0) {}

Client::Client(int id, const QString &nom, const QString &prenom, const QString &adresse, const QString &telephone)
    : id(id), nom(nom), prenom(prenom), adresse(adresse), telephone(telephone) {}

int Client::getId() const { return id; }
QString Client::getNom() const { return nom; }
QString Client::getPrenom() const { return prenom; }
QString Client::getAdresse() const { return adresse; }
QString Client::getTelephone() const { return telephone; }

void Client::setNom(const QString &nom) { this->nom = nom; }
void Client::setPrenom(const QString &prenom) { this->prenom = prenom; }
void Client::setAdresse(const QString &adresse) { this->adresse = adresse; }
void Client::setTelephone(const QString &telephone) { this->telephone = telephone; }
