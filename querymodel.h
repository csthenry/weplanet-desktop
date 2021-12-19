#ifndef QUERYMODEL_H
#define QUERYMODEL_H

#include <QItemSelectionModel>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlError>
#include <QSql>

class queryModel
{
public:
    queryModel(QSqlDatabase db, QObject* parent);
    QSqlQueryModel* setBaseUserInfo();
    QSqlTableModel* setActGroupPage_departmentModel();
    QSqlTableModel* setActGroupPage_groupModel();
    QSqlRelationalTableModel* setActAttendPage_relationalTableModel();
    QSqlRelationalTableModel *setActUserPage_relationalTableModel();
private:
    QString uid;
    QObject* parent;    //主窗口
    QSqlDatabase db;
    QSqlQueryModel *qryModel;
    QSqlTableModel *tabModel;
    QSqlRelationalTableModel *relTableModel;
    QItemSelection *theSelection;
};

#endif // QUERYMODEL_H
