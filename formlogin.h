/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#ifndef FORMLOGIN_H
#define FORMLOGIN_H

#include <QDialog>
#include <QMessageBox>
#include "service.h"

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
    QString readPwd;    //保存的密码
    QString loginUid;   //登录成功的uid
    void writeLoginSettings();
    QString readLoginSettings();
    bool autoLoginSuccess = false;  //自动登录标记

public:
    bool autoLogin();   //获取自动登录鉴权信息

private slots:
    void on_btn_Login_clicked();

    void on_checkBox_remPwd_clicked(bool checked);

    void on_lineEdit_Uid_textEdited(const QString &arg1);

    void on_btn_Signup_clicked();

    void on_lineEdit_Pwd_textEdited(const QString &arg1);

private:
    Ui::formLogin *ui;
    QSqlDatabase db;

signals:
    void sendData(QSqlDatabase db, QString uid);
};

#endif // FORMLOGIN_H
