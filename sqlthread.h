#ifndef SQLTHREAD_H
#define SQLTHREAD_H

#include <QThread>
#include <QtSql>
#include <QSqlQuery>
#include "service.h"

class SqlThread : public QThread
{
    Q_OBJECT
private:
    int cnt = 1;
    QSqlDatabase db;
    service dbService;
    QSqlQuery *testDbConnection;
    bool status = false;
    bool isPaused = true;
    bool isStop = true; //停止线程
    bool dbStatus = false;  //心跳验证状态

protected:
    void run() Q_DECL_OVERRIDE;

public:
    SqlThread();
    void beginThread();
    void stopThread();
    bool getisPaused();
    QSqlDatabase getDb();

signals:
    void newStatus(bool status);
    void firstFinished();

};

#endif // SQLTHREAD_H
