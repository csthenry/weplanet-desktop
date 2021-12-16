#ifndef QUERYMODEL_H
#define QUERYMODEL_H

#include <QItemSelectionModel>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSql>

class queryModel
{
public:
    queryModel(QSqlDatabase db, QObject* parent);
    QSqlQueryModel* setBaseUserInfo();
    QSqlTableModel* setActGroupPage_departmentModel();
    QSqlTableModel* setActGroupPage_groupModel();
private:
    QString uid;
    QObject* parent;    //主窗口
    QSqlDatabase db;
    QSqlQueryModel *qryModel;
    QSqlTableModel *tabModel;
    QItemSelection *theSelection;
};

#endif // QUERYMODEL_H
