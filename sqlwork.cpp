#include "sqlwork.h"

void SqlWork::working()
{
    run = true;
    while(run)
    {
        while(!isStop)
        {
            testDbConnection = new QSqlQuery(testDB);
            dbStatus = testDbConnection->exec("select version();");
            if(!dbStatus)
                isPaused = false;
            delete testDbConnection;
            if(isPaused)
            {
                QThread::sleep(5);
                continue;
            }
            qDebug() << "SqlThread线程运行中,db:" << dbStatus << this->thread();
            if(!dbStatus)
                testDB.open();
            if(!dbStatus && !DB.open())
                status = false;
            else
            {
                status = true;
                isPaused = true;    //连接成功后暂停线程
                if(cnt++ == 1)
                    emit firstFinished();
            }
            emit newStatus(status);
            QThread::sleep(5);
        }
    }
}

SqlWork::SqlWork(QString dbName)
{
    this->dbName = dbName;
    service dbService;
    dbService.addDatabase(DB, this->dbName);
    dbService.addDatabase(testDB, "test_" + this->dbName);
    moveToThread(this->thread());
}

void SqlWork::beginThread()   //完成数据库查询后调用此函数继续对数据库心跳验证
{
    isStop = false;
}

void SqlWork::stopThread()    //在对数据库查询时请调用此函数暂停该线程，避免冲突
{
    isStop = true;
}

void SqlWork::quit()
{
    run = false;
}

bool SqlWork::getisPaused()
{
    return isPaused;
}

QSqlDatabase SqlWork::getDb()
{
    return DB;
}

QString SqlWork::getDbName()
{
    return dbName;
}

