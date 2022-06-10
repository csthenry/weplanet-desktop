#pragma once
#pragma execution_character_set("utf-8")

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
    void initDatabaseTables();
    void bindQQAvatar(QString qqNumber);   //tag==0该邮箱不是qq邮箱，tag==1获取头像成功，tag==-1其他错误
    void updateScore(float score);

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
    QString getScore();
    QPixmap getAvatar();

private slots:
    void autoAuthAccount(const long long account, const QString& pwd);
    void authAccount(const long long account, const QString& pwd, const QString& editPwd);
    void setAuthority(const QString &uid);
    void signUp(const QString& pwd, const QString& name, const QString& tel, const QString& gender);
    void editPersonalInfo(const QString& oldPwd, const QString& tel, const QString& mail, const QString& avatar, const QString& pwd);
private:
    bool isAttend;
    QDateTime curDateTime;
    service db_service;
    QSqlDatabase DB, initDB;
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
    QString score;
    QPixmap avatar;
    QString attendBeginTime, attendEndTime;
    QPixmap loadAvatar(const QString& url);
    QString loadGroup(const QString& uid);
    QString loadDepartment(const QString& uid);

signals:
    void baseInfoFinished();
    void authRes(int);     //返回账号验证结果
    void autoAuthRes(int);
    void signupRes(bool);   //返回注册结果
    void authorityRes(QSqlRecord);    //返回账号权限鉴权结果
    void editPersonalInfoRes(int);  //个人信息修改结果：1修改成功（不包含密码），2修改成功（包含密码，需要注销），-1修改失败（旧密码验证失败）
    void initDatabaseFinished(bool);
    void bindQQAvatarFinished(int);
};

#endif // BASEINFOWORK_H
