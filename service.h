#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QtSql>
#include <QCryptographicHash>

class service
{
public:
    static QString pwdEncrypt(const QString& str);//字符串加密

    static void connectDatabase(QSqlDatabase& db);

    static bool initDatabaseTables(QSqlDatabase& db);

    static bool authAccount(QSqlDatabase& db, int uid, QString pwd);

    service();

private:

};

#endif // SERVICE_H
