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
    static QString pwdEncrypt(const QString& str);  //字符串加密

    void connectDatabase(QSqlDatabase& db);

    static bool initDatabaseTables(QSqlDatabase& db);

    static bool authAccount(QSqlDatabase& db, QString& uid, const long long account, const QString& pwd);

    static QPixmap getAvatar(const QString& url);

    static QPixmap setAvatarStyle(QPixmap);

    static QString getGroup(const QString& uid);

    static QString getDepartment(const QString& uid);

    service();

private:

    QSqlDatabase db;
    QString dataBaseType;
    QString hostName;
    int dataBasePort;
    QString dataBaseName;
    QString dataBaseUserName;
    QString dataBasePassword;

};

#endif // SERVICE_H
