/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "mainwindow.h"
#include "formlogin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    formLogin *formLoginWindow = new formLogin();

    if (formLoginWindow->autoLoginSuccess || formLoginWindow->exec() == QDialog::Accepted)
    {
        MainWindow w(nullptr, formLoginWindow);
        w.show();
        formLoginWindow->send();    //发送信号
        delete formLoginWindow;
        return a.exec();
    }
    else
    {
        delete formLoginWindow;
        return  0;
    }
    return a.exec();
}
