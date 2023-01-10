#pragma once
#pragma execution_character_set("utf-8")

#ifndef ATTENDWORK_H
#define ATTENDWORK_H

#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QJsonArray>
#include "service.h"
#include "querymodel.h"

class AttendWork : public QObject
{
    Q_OBJECT
public:
    explicit AttendWork(QObject *parent = nullptr);
    void working();
    void homeChartWorking();
    void analyseWorkTime();
    void analyseWorkStatus();
    QSqlRecord getRecord(const int index);
    QSqlDatabase getDB();
    void setUid(const QString& uid);
    void setModel(QSqlRelationalTableModel *relTableModel);
    int fieldIndex(const QString &field);
    int* getWorkTime();
    QJsonArray getWeekMyWorkTime();
    QJsonArray getWeekAllWorkStatus();
    QJsonArray getWeekWorkMem();
private slots:
    void submitAll(int type);   //1代表签到，0代表签退
private:
    int workTimeData[4] = {0};
    service db_service;
    QString uid;
    QSqlDatabase DB, DB_SECOND;
    QSqlRelationalTableModel *relTableModel;
    QJsonArray weekMyWorkTime, weekAllWorkStatus, weekWorkMem;
signals:
    void attendWorkFinished();
    void attendDone(bool);
    void attendOutDone(bool);
	void homeChartDone(QString);
};

#endif // ATTENDWORK_H
