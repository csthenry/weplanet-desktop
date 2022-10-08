/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/
#pragma once
#pragma execution_character_set("utf-8")

#ifndef FORMLOGIN_H
#define FORMLOGIN_H

#include <QDialog>
#include <QMessageBox>
#include <QThread>
#include <QTime>
#include "sqlwork.h"
#include "baseinfowork.h"
#include "checkupdate.h"

namespace Ui {
class formLogin;
}

class formLogin : public QDialog
{
    Q_OBJECT

public:
    void send();
    explicit formLogin(QDialog *parent = nullptr);
    ~formLogin();

private:
    bool isAutoLogin;
    bool isQuit = false;    //标记线程已停止
    int loginErrCnt = 0;
    QTime curTime;
    QSettings* config_ini;
    SqlWork *sqlWork;
    baseInfoWork *loginWork;
    QThread *sqlThread, *dbThread;

    QMovie *loadingMovie, *loadingMovie_2;
    QString readPwd;    //保存的密码
    QString loginUid;   //登录成功的uid
    void writeLoginSettings();
    void beforeAccept();
    QString readLoginSettings();
    bool autoLoginSuccess = false;  //自动登录标记
    bool dbStatus = true;
    QPixmap *statusOKIcon, *statusErrorIcon;
    checkUpdate *updateSoftWare;
    void updateFinished(bool res);

public:
    bool autoLogin();   //获取自动登录鉴权信息

private slots:

    void on_btn_Login_clicked();

    void on_checkBox_remPwd_clicked(bool checked);

    void on_lineEdit_Uid_textEdited(const QString &arg1);

    void on_btn_Signup_clicked();

    void on_lineEdit_Pwd_textEdited(const QString &arg1);

    void on_statusChanged(const bool status);

    void on_authAccountRes(int res);

    void on_autoLoginAuthAccountRes(int res);

    void on_signUpFinished(bool res);

private:
    Ui::formLogin *ui;

signals:
    void sendData(QString uid);
    void startDbWork();
    void authAccount(const long long account, const QString& pwd, const QString& editPwd);
    void autoLoginAuthAccount(const long long account, const QString& pwd);
    void signUp(const QString& pwd, const QString& name, const QString& tel, const QString& gender);
    void initDatabase();
    void beginUpdate();
	void getAnnouncement();
};

#endif // FORMLOGIN_H
