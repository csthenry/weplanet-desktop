#include "activitymanagework.h"

ActivityManageWork::ActivityManageWork(QObject *parent)
	: QObject(parent)
{
    db_service.addDatabase(DB, "ActivityManageWork_DB");
}

ActivityManageWork::~ActivityManageWork()
{
}

void ActivityManageWork::working()
{
    DB.open();
    if (!isFirst)
    {
        tabModel->select();
        emit activityManageWorkFinished();
        return;
    }
    tabModel->setTable("magic_activity");
    tabModel->setSort(tabModel->fieldIndex("act_id"), Qt::AscendingOrder);
    tabModel->setEditStrategy(QSqlTableModel::OnRowChange);
    tabModel->setHeaderData(tabModel->fieldIndex("act_id"), Qt::Horizontal, "编号");
    tabModel->setHeaderData(tabModel->fieldIndex("act_name"), Qt::Horizontal, "活动名称");
    tabModel->setHeaderData(tabModel->fieldIndex("act_des"), Qt::Horizontal, "活动描述");
    tabModel->setHeaderData(tabModel->fieldIndex("joinDate"), Qt::Horizontal, "报名时间");
    tabModel->setHeaderData(tabModel->fieldIndex("beginDate"), Qt::Horizontal, "开始时间");
    tabModel->setHeaderData(tabModel->fieldIndex("endDate"), Qt::Horizontal, "结束时间");
    tabModel->setHeaderData(tabModel->fieldIndex("editUid"), Qt::Horizontal, "发布者UID");
    tabModel->select();

    isFirst = false;
    emit activityManageWorkFinished();
}

QSqlDatabase ActivityManageWork::getDB()
{
    return DB;
}

void ActivityManageWork::submitAll()
{
    bool res = true;
    if(tabModel->isDirty())
		res = tabModel->submitAll();
    emit submitAllFinished(res);
}

void ActivityManageWork::setModel(QSqlTableModel* model)
{
    tabModel = model;
}