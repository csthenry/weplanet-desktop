#include "posterwork.h"
#include <QDebug>

PosterWork::PosterWork(QObject *parent)
	: QObject(parent)
{
    db_service.addDatabase(DB, "PosterManageWork_DB");
}

PosterWork::~PosterWork()
{
}

void PosterWork::working()
{
    DB.open();
    if (workType == -1)
        return;
    if (workType != 1)
    {
        tabModel->setTable("magic_contents");
        tabModel->setSort(tabModel->fieldIndex("c_id"), Qt::DescendingOrder);
        tabModel->setHeaderData(tabModel->fieldIndex("c_id"), Qt::Horizontal, "编号");
        tabModel->setHeaderData(tabModel->fieldIndex("title"), Qt::Horizontal, "标题");
        tabModel->setHeaderData(tabModel->fieldIndex("author_id"), Qt::Horizontal, "发布者");
        tabModel->select();
        
        emit contentsWorkFinished();
    }
    else
    {
        manageModel->setTable("magic_contents");
        manageModel->setSort(manageModel->fieldIndex("c_id"), Qt::DescendingOrder);
        manageModel->setEditStrategy(QSqlTableModel::OnRowChange);
        manageModel->setHeaderData(manageModel->fieldIndex("c_id"), Qt::Horizontal, "编号");
        manageModel->setHeaderData(manageModel->fieldIndex("title"), Qt::Horizontal, "标题");
        manageModel->setHeaderData(manageModel->fieldIndex("author_id"), Qt::Horizontal, "发布者");
        manageModel->select();
        qDebug() << "posterWork完成";
        emit contentsManageWorkFinished();
    }
}

QSqlDatabase PosterWork::getDB()
{
    return DB;
}

void PosterWork::submitAll()
{
    manageModel->submitAll();
}

void PosterWork::setModel(QSqlTableModel* model)
{
    tabModel = model;
}

void PosterWork::setManageModel(QSqlTableModel* model)
{
    manageModel = model;
}

void PosterWork::setWorkType(int type)
{
    workType = type;
}
