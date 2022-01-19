#ifndef ATTENDWORK_H
#define ATTENDWORK_H

#include <QObject>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlQuery>
#include "querymodel.h"

class AttendWork : public QObject
{
    Q_OBJECT
public:
    explicit AttendWork(QObject *parent = nullptr);
    void working();
    void analyseWorkTime();
    QSqlRecord getRecord(const int index);
    void setDB(const QSqlDatabase& db);
    void setUid(const QString& uid);
    void setModel(QSqlRelationalTableModel *relTableModel);
    int fieldIndex(const QString &field);
    int* getWorkTime();
private slots:
    void submitAll(int type);   //1代表签到，0代表签退
private:
    int workTimeData[4] = {0};
    QString uid;
    QSqlDatabase DB;
    QSqlRelationalTableModel *relTableModel;
signals:
    void attendWorkFinished();
    void attendDone(bool);
    void attendOutDone(bool);
};

#endif // ATTENDWORK_H
