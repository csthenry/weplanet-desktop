#ifndef BASEINFOWORK_H
#define BASEINFOWORK_H

#include <QObject>
#include <QThread>
#include <QtSql>
#include <QDateTime>
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

    bool getAttendToday();
    QString getBeginTime();
    QString getEndTime();
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
    void setAuthority(const QString &uid, const QVector<QAction *> &vector);
    void signUp(const QString& pwd, const QString& name, const QString& tel);
    void editPersonalInfo(const QString& oldPwd, const QString& tel, const QString& mail, const QString& avatar, const QString& pwd);
private:
    bool isAttend;
    QDateTime curDateTime;
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
    QString attendBeginTime, attendEndTime;
    QPixmap loadAvatar(const QString& url);
    QString loadGroup(const QString& uid);
    QString loadDepartment(const QString& uid);

signals:
    void baseInfoFinished();
    void authRes(bool);     //返回账号验证结果
    void autoAuthRes(bool);
    void signupRes(bool);   //返回注册结果
    void authorityRes(bool);    //返回账号权限鉴权结果
    void editPersonalInfoRes(int);  //个人信息修改结果：1修改成功（不包含密码），2修改成功（包含密码，需要注销），-1修改失败（旧密码验证失败）
};

#endif // BASEINFOWORK_H
