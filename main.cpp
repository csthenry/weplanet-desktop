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
    //Windows平台用来获取屏幕分辨率及缩放比例

    //分辨率 GetSystemMetrics获取系统属性
    int ScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	//屏幕缩放 GetDeviceCaps获取设备的物理属性
    HWND hwd = ::GetDesktopWindow();
    HDC hdc = ::GetDC(hwd);
    int width = GetDeviceCaps(hdc, DESKTOPHORZRES);
    int height = GetDeviceCaps(hdc, DESKTOPVERTRES);
    double dWidth = (double)width;
    double dScreenWidth = (double)ScreenWidth;
    double scale = dWidth / dScreenWidth;

    qDebug() << "width:" << width << ",ScreenWidth:" << ScreenWidth << ",height:" << height << ",scale:" << scale;

    //程序UI基于125%缩放开发
	//if(scale >= 1.25)
 //       qputenv("QT_SCALE_FACTOR", "1.0");  //根据系统缩放比例调整
 //   else
 //       qputenv("QT_SCALE_FACTOR", QString::number(scale + 0.25).toLatin1());
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
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
