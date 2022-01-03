/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

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
    queryModel(QSqlDatabase db, QObject* parent);
    QSqlTableModel* setActGroupPage_departmentModel();
    QSqlTableModel* setActGroupPage_groupModel();
    QSqlTableModel* setActivityPage();
    QSqlRelationalTableModel* setActAttendPage_relationalTableModel();
    QSqlRelationalTableModel *setActUserPage_relationalTableModel();
    void analyseWorkTime(int& data_1, int& data_2, int& data_3, int& data_4);
private:
    QString uid;
    QObject* parent;    //主窗口
    QSqlDatabase db;
    QSqlTableModel *tabModel;
    QSqlRelationalTableModel *relTableModel;
    QItemSelection *theSelection;
};

#endif // QUERYMODEL_H
