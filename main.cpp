/***************************************************/
/*                 MagicLitePlanet                 */
/* Copyright (c) 2017-2022 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "mainwindow.h"
#include "formlogin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);   //开启高缩放支持图片
	
    QApplication a(argc, argv);
	
    formLogin *formLoginWindow = new formLogin();

    if(formLoginWindow->exec() == QDialog::Accepted)
    {
        MainWindow w(nullptr, formLoginWindow);
        formLoginWindow->send();    //发送登录信息信号
        delete formLoginWindow;
        w.show();
        return a.exec();
    }
    delete formLoginWindow;
    return 0;
}
