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
    void loadBaseInfoWorking(QString uid);
    void loadAvatarPic(QString url);

    QString getName();
    QString getGender();
    QString getTel();
    QString getMail();
    QPixmap getAvatar();

private:
    QString name;
    QString gender;
    QString telephone;
    QString mail;
    QString group;
    QString department;
    QString avatarUrl;
    QPixmap avatar;
    QPixmap loadAvatar(const QString& url);

signals:
    void baseInfoFinished();
//    void sendName(QString name);
//    void sendGender(QString gender);
//    void sendTel(QString telephone);
//    void sendMail(QString mail);
//    void sendGroup(QString group);
//    void sendDpt(QString department);
//    void sendAvatar(QString avatar);
};

#endif // BASEINFOWORK_H
