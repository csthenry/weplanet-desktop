#include "sqlwork.h"

void SqlWork::working()
{
    run = true;
    while(run)
    {
        while(!isStop)
        {
            if(!dbStatus)
                isPaused = false;
            if(!dbStatus || !testDB.isOpen())
				testDB.open();
            testDbConnection = new QSqlQuery(testDB);
            dbStatus = testDbConnection->exec("select version();");
            delete testDbConnection;
            if(isPaused)
            {
                QThread::sleep(5);
                continue;
            }
            if(!dbStatus)
	            status = false;
            else
            {
	            if(dbName != "mainDB" && !DB.isOpen())
		            DB.open();
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
    if(dbName == "mainDB")
    {
        dbService.addDatabase(testDB, "test_" + this->dbName);  //如果是主窗口，则该类的DB弃用，DB由各个工作对象自行添加并连接
    }
    else
    {
        dbService.addDatabase(DB, this->dbName);
        dbService.addDatabase(testDB, "test_" + this->dbName);
    }
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

QSqlDatabase SqlWork::getTestDb()
{
    return testDB;
}

QString SqlWork::getDbName()
{
    return dbName;
}

