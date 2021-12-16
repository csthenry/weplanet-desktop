#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QtSql>
#include <QSettings>
#include <QPainter>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class service
{
public:
    static QString pwdEncrypt(const QString& str);//字符串加密

    static void connectDatabase(QSqlDatabase& db);

    static bool initDatabaseTables(QSqlDatabase& db);

    static bool authAccount(QSqlDatabase& db, QString& uid, long long account, QString pwd);

    static QPixmap getAvatar(QString url);

    static QPixmap setAvatarStyle(QPixmap);

    static QString getGroup(QString uid);

    static QString getDepartment(QString uid);

    service();

private:

};

#endif // SERVICE_H
