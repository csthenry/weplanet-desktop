#ifndef BASEINFOWORK_H
#define BASEINFOWORK_H

#include <QObject>
#include <QThread>
#include <QtSql>
#include "service.h"

class baseInfoWork : public QObject
{
    Q_OBJECT
public:
    explicit baseInfoWork(QObject *parent = nullptr);
    void loadBaseInfoWorking();
    void loadAvatarPic(QString url);

    void refreshBaseInfo(); //用于刷新数据
    void setUid(QString uid);
    void setDB(const QSqlDatabase &DB);

    QString getName();
    QString getGender();
    QString getTel();
    QString getMail();
    QString getGroup();
    QString getDepartment();
    QPixmap getAvatar();
public slots:

private:
    QSqlDatabase DB;
    QString uid;
    QString name;
    QString gender;
    QString telephone;
    QString mail;
    QString group;
    QString department;
    QString avatarUrl;
    QPixmap avatar;
    QPixmap loadAvatar(const QString& url);
    QString loadGroup(const QString& uid);
    QString loadDepartment(const QString& uid);

signals:
    void baseInfoFinished();
};

#endif // BASEINFOWORK_H
