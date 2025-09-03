#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>

QSqlDatabase Database::db()
{
    return QSqlDatabase::database();
}

bool Database::initialize()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("lms.db");
    if (!db.open())
        return false;
    QSqlQuery query;
    // Create tables if not exist
    query.exec("CREATE TABLE IF NOT EXISTS client (id INTEGER PRIMARY KEY AUTOINCREMENT, nom TEXT, prenom TEXT, adresse TEXT, telephone TEXT)");
    query.exec("CREATE TABLE IF NOT EXISTS commande (id INTEGER PRIMARY KEY AUTOINCREMENT, client TEXT, etat TEXT, date TEXT, montant REAL)");
    return true;
}
