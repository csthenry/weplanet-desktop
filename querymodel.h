/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/
#pragma once
#pragma execution_character_set("utf-8")


#ifndef QUERYMODEL_H
#define QUERYMODEL_H

#include <QItemSelectionModel>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlError>
#include <QTime>
#include <QSql>

class queryModel
{
public:
    queryModel();
    queryModel(QSqlDatabase db, QObject* parent = nullptr);
    QSqlTableModel* setActGroupPage_departmentModel();
    QSqlTableModel* setActGroupPage_groupModel();
    QSqlTableModel* setActivityPage();
    QSqlRelationalTableModel* setActAttendPage_relationalTableModel();
    QSqlRelationalTableModel* setActUserPage_relationalTableModel();
    void analyseWorkTime(int data[]);

    QSqlTableModel* getTableModel();
    QSqlRelationalTableModel* getrelTableModel();
private:
    QString uid;
    QObject* parent;
    QSqlDatabase db;
    QSqlTableModel *tabModel;
    QSqlRelationalTableModel *relTableModel;
};

#endif // QUERYMODEL_H
