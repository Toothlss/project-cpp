#ifndef CLIENT_H
#define CLIENT_H
#include <QString>

class Client
{
public:
    Client();
    Client(int id, const QString &nom, const QString &prenom, const QString &adresse, const QString &telephone);

    int getId() const;
    QString getNom() const;
    QString getPrenom() const;
    QString getAdresse() const;
    QString getTelephone() const;

    void setNom(const QString &nom);
    void setPrenom(const QString &prenom);
    void setAdresse(const QString &adresse);
    void setTelephone(const QString &telephone);

private:
    int id;
    QString nom;
    QString prenom;
    QString adresse;
    QString telephone;
};

#endif // CLIENT_H
