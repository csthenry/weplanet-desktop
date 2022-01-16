#include "sqlthread.h"

void SqlThread::run()
{
    while(!isStop)
    {
        testDbConnection = new QSqlQuery(db);
        dbStatus = testDbConnection->exec("select version();");
        delete testDbConnection;
        if(isPaused)
        {
            sleep(5);
            continue;
        }
        qDebug() << "SqlThread线程运行中,db:" << dbStatus;
        if(!dbStatus && !db.open())
            status = false;
        else
        {
            status = true;
            isPaused = true;    //连接成功后暂停线程
            if(cnt++ == 1)
                emit firstFinished();
        }
        emit newStatus(status);
        sleep(5);
    }
    quit();
}

SqlThread::SqlThread()
{
    dbService.connectDatabase(db);
    moveToThread(this);     //线程中执行
}

void SqlThread::beginThread()   //完成数据库查询后调用此函数继续对数据库心跳验证
{
    isStop = false;
    isPaused = false;
}

void SqlThread::stopThread()    //在对数据库查询时请调用此函数暂停该线程，避免冲突
{
    isStop = true;
}

bool SqlThread::getisPaused()
{
    return isPaused;
}

QSqlDatabase SqlThread::getDb()
{
    return db;
}

