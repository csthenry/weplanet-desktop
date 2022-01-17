#ifndef SQLWORK_H
#define SQLWORK_H

#include <QtSql>
#include <QThread>
#include <QSqlQuery>
#include "service.h"

class SqlWork : public QObject
{
    Q_OBJECT
private:
    int cnt = 1;
    QSqlDatabase DB;
    QString dbName;
    service dbService;
    QSqlQuery *testDbConnection;
    bool run = true;
    bool status = false;
    bool isPaused = true;
    bool isStop = true; //停止线程
    bool dbStatus = false;  //心跳验证状态

public:
    SqlWork(QString dbName);
    void working();
    void beginThread();
    void stopThread();
    void quit();    //用于结束working函数，结束后需重新emit触发working
    bool getisPaused();
    QSqlDatabase getDb();
    QString getDbName();

signals:
    void newStatus(bool status);
    void firstFinished();

};

#endif // SQLWORK_H
