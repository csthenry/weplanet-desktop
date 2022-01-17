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
    service dbService;
    QSqlQuery *testDbConnection;
    bool status = false;
    bool isPaused = true;
    bool isStop = true; //停止线程
    bool dbStatus = false;  //心跳验证状态

public:
    SqlWork();
    void working();
    void beginThread();
    void stopThread();
    bool getisPaused();
    QSqlDatabase getDb();

signals:
    void newStatus(bool status);
    void firstFinished();

};

#endif // SQLWORK_H
