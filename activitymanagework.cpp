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
        memberTabModel->select();
        emit activityManageWorkFinished(type);
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

    memberTabModel->setTable("magic_activityMembers");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("actm_id"), Qt::Horizontal, "报名号");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("actm_uid"), Qt::Horizontal, "用户UID");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("actm_joinDate"), Qt::Horizontal, "报名时间");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("status"), Qt::Horizontal, "录取状态");
    memberTabModel->select();

    isFirst = false;
    emit activityManageWorkFinished(type);
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

void ActivityManageWork::setMemberModel(QSqlTableModel* model)
{
    memberTabModel = model;
}

void ActivityManageWork::apply(QString& uid)
{
}

void ActivityManageWork::setType(int t)
{
    type = t;
}
