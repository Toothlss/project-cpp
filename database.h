#ifndef DATABASE_H
#define DATABASE_H
#include <QSqlDatabase>

class Database
{
public:
    static bool initialize();
    static QSqlDatabase db();
};

#endif // DATABASE_H
