#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QtSql>
#include <QSettings>
#include <QCryptographicHash>

class service
{
public:
    static QString pwdEncrypt(const QString& str);//字符串加密

    static void connectDatabase(QSqlDatabase& db);

    static bool initDatabaseTables(QSqlDatabase& db);

    static bool authAccount(QSqlDatabase& db, QString& uid, int account, QString pwd);

    service();

private:

};

#endif // SERVICE_H
