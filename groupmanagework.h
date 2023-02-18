#pragma once
#pragma execution_character_set("utf-8")

#ifndef GROUPMANAGEWORK_H
#define GROUPMANAGEWORK_H

#include <QObject>
#include <QMutex>
#include <QSqlQuery>
#include "service.h"
#include "querymodel.h"

class GroupManageWork : public QObject
{
    Q_OBJECT
public:
    explicit GroupManageWork(QObject *parent = nullptr);
    ~GroupManageWork();
    void working();
    void setGroupModel(QSqlTableModel* group);
    QSqlDatabase getDB();
    void setDepartmentModel(QSqlTableModel* department);
    void submitAll(int type);   //1代表提交用户组model，0为部门
    void fixUser(int type, const QString& removedId);
private:
    service db_service;
    QSqlDatabase DB, DB_SECOND;
    QSqlTableModel *groupModel, *departmentModel;

signals:
    void groupManageWorkFinished();
    void submitFinished_0(bool);
    void submitFinished_1(bool);
};

#endif // GROUPMANAGEWORK_H
