﻿/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/
#pragma once
#pragma execution_character_set("utf-8")

#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QAction>
#include <QPainterPath>
#include <QUdpSocket>
#include <QSettings>
#include <QPainter>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <./include/db/db.h>  //数据库服务
#include <./smtp/SmtpMime>  //邮件服务 https://github.com/bluetiger9/SmtpClient-for-Qt
#include <./wintoast/wintoastlib.h> //系统推送服务 https://github.com/mohabouje/WinToast https://www.codeproject.com/Articles/5286393/Cplusplus-Windows-Toast-Notification

class service
{
public:
    static qint32 getWebTime(); //网络时间，返回时间戳，错误返回-1
    
    static QString pwdEncrypt(const QString& str);  //字符串加密

    void connectDatabase(QSqlDatabase& db);
    
    void addDatabase(QSqlDatabase& db, const QString &flag);

    static bool initDatabaseTables(QSqlDatabase db);

    //验证返回状态码：200验证成功；400账号封禁；403账号密码不匹配；500网络错误
    static int authAccount(QSqlDatabase& db, QString& uid, const long long account, const QString& pwd);

    static bool setAuthority(QSqlDatabase& db, const QString& uid, const QVector<QAction *> &vector);

    static QPixmap getAvatar(const QString& url);

    static QPixmap setAvatarStyle(QPixmap);

    static QString getGroup(QSqlDatabase& db, const QString& uid);

    static QString getDepartment(QSqlDatabase& db, const QString& uid);

    static int sendMail(const QList<QString> smtp_config, const QString& mailto, const QString& title, const QString& mailtext);

    //返回目录占用空间大小，单位KB
    static float getDirSize(const QString& dirPath);

    //删除目录下所有文件
    static int deleteDir(const QString& dirPath);

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
