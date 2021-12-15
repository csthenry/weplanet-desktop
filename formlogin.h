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

private slots:
    void on_btn_Login_clicked();

    void on_checkBox_remPwd_clicked(bool checked);

    void on_lineEdit_Uid_textEdited(const QString &arg1);

    void on_btn_Signup_clicked();

private:
    Ui::formLogin *ui;
    QSqlDatabase db;

signals:
    void sendData(QSqlDatabase db, QString uid);
};

#endif // FORMLOGIN_H
