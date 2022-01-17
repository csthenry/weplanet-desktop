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

    QString getLoginUid();
    QString getLastSignupUid();
    QString getName();
    QString getGender();
    QString getTel();
    QString getMail();
    QString getGroup();
    QString getDepartment();
    QPixmap getAvatar();
public slots:
    void autoAuthAccount(const long long account, const QString& pwd);
    void authAccount(const long long account, const QString& pwd, const QString& editPwd);
    void setAuthority(QString &uid, QVector<QAction*>& vector);
    void signUp(const QString& pwd, const QString& name, const QString& tel);
private:
    QSqlDatabase DB;
    QString uid;
    QString loginUid;
    QString lastSignupUid;
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
    void authRes(bool);     //返回账号验证结果
    void autoAuthRes(bool);
    void signupRes(bool);   //返回注册结果
    void authorityRes(bool);    //返回账号权限鉴权结果
};

#endif // BASEINFOWORK_H
