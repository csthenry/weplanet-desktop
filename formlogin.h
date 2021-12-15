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

private slots:
    void on_btn_Login_clicked();

private:
    Ui::formLogin *ui;
    QSqlDatabase db;

signals:
    void sendData(QSqlDatabase db, QString uid);
};

#endif // FORMLOGIN_H
