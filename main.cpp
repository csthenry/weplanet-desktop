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

//日志文件
void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	Q_UNUSED(context);
	QMutex m_LogMutex;
	QString txt;
    switch (type)
    {
	case QtDebugMsg:
		txt = QString("[%2][Info] %1").arg(msg, QDateTime::currentDateTime().toString("HH:mm:ss"));
		break;
	case QtWarningMsg:
		txt = QString("[%2][Warning] %1").arg(msg, QDateTime::currentDateTime().toString("HH:mm:ss"));
		break;
	case QtCriticalMsg:
		txt = QString("[%2][Critical] %1").arg(msg, QDateTime::currentDateTime().toString("HH:mm:ss"));
		break;
	case QtFatalMsg:
		txt = QString("[%2][Fatal] %1").arg(msg, QDateTime::currentDateTime().toString("HH:mm:ss"));
		abort();
	}

	m_LogMutex.lock();
	QFile outFile(QString("%1/log/%2.log").arg(QDir::currentPath(), QDateTime::currentDateTime().toString("yyyy-MM-dd")));
	outFile.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream ts(&outFile);
	ts << txt << endl;
	outFile.close();
	m_LogMutex.unlock();
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);   //开启高缩放支持图片
    QApplication a(argc, argv);

	#ifdef QT_NO_DEBUG  
	qInstallMessageHandler(customMessageHandler);   //release模式下注册日志处理函数
	#endif

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
