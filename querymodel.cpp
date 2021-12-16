#include "querymodel.h"
#include <QtDebug>
#include "service.h"

queryModel::queryModel(QSqlDatabase db, QObject* parent)
{
    this->db = db;
    this->parent = parent;
}

QSqlQueryModel* queryModel::setBaseUserInfo()
{

    qryModel = new QSqlQueryModel(parent);
    if(!db.open())
        return nullptr;
    else
    {
        qDebug() << "进入查询 baseUserInfo";
        //qryModel->setQuery("SELECT name, gender, telephone, mail, user_group, user_position, user_avatar FROM magic_users WHERE uid = " + uid + ";");
    }
    if(qryModel->lastError().isValid())
        return nullptr;
    return qryModel;
}

QSqlTableModel *queryModel::setActGroupPage_departmentModel()
{
    tabModel = new QSqlTableModel(parent, db);

    tabModel->setTable("magic_department");
    tabModel->setSort(tabModel->fieldIndex("dpt_id"), Qt::AscendingOrder);
    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    tabModel->setHeaderData(tabModel->fieldIndex("dpt_id"),Qt::Horizontal,"编号");
    tabModel->setHeaderData(tabModel->fieldIndex("dpt_name"),Qt::Horizontal,"部门名称");
    if(!tabModel->select())
        return nullptr;

    if(tabModel->lastError().isValid())
        return nullptr;
    return tabModel;
}

QSqlTableModel *queryModel::setActGroupPage_groupModel()
{
    tabModel = new QSqlTableModel(parent, db);

    tabModel->setTable("magic_group");
    tabModel->setSort(tabModel->fieldIndex("group_id"), Qt::AscendingOrder);
    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    tabModel->setHeaderData(tabModel->fieldIndex("group_id"),Qt::Horizontal,"编号");
    tabModel->setHeaderData(tabModel->fieldIndex("group_name"),Qt::Horizontal,"用户组名称");
    tabModel->setHeaderData(tabModel->fieldIndex("users_manage"),Qt::Horizontal,"用户管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("attend_manage"),Qt::Horizontal,"考勤管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("apply_manage"),Qt::Horizontal,"审批权限");
    tabModel->setHeaderData(tabModel->fieldIndex("applyItem_manage"),Qt::Horizontal,"审批流程管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("group_manage"),Qt::Horizontal,"团体架构管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("activity_manage"),Qt::Horizontal,"活动管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("send_message"),Qt::Horizontal,"信息收发权限");
    if(!tabModel->select())
        return nullptr;

    if(tabModel->lastError().isValid())
        return nullptr;
    return tabModel;
}
