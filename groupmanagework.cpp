#include "groupmanagework.h"

GroupManageWork::GroupManageWork(QObject *parent) : QObject(parent)
{

}

void GroupManageWork::working()
{
    if(!isFirst)
    {
        groupModel->select();
        departmentModel->select();

        emit groupManageWorkFinished();
        return;
    }
    groupModel->setTable("magic_group");
    groupModel->setSort(groupModel->fieldIndex("group_id"), Qt::AscendingOrder);    //升序排列
    groupModel->setEditStrategy(QSqlTableModel::OnManualSubmit);  //手动提交
    groupModel->setHeaderData(groupModel->fieldIndex("group_id"), Qt::Horizontal,"编号");
    groupModel->setHeaderData(groupModel->fieldIndex("group_name"), Qt::Horizontal,"用户组名称");
    groupModel->setHeaderData(groupModel->fieldIndex("users_manage"), Qt::Horizontal,"用户管理权");
    groupModel->setHeaderData(groupModel->fieldIndex("attend_manage"), Qt::Horizontal,"考勤管理权");
    groupModel->setHeaderData(groupModel->fieldIndex("apply_manage"), Qt::Horizontal,"审批权限");
    groupModel->setHeaderData(groupModel->fieldIndex("applyItem_manage"), Qt::Horizontal,"审批流程管理权");
    groupModel->setHeaderData(groupModel->fieldIndex("group_manage"), Qt::Horizontal,"组织架构管理权");
    groupModel->setHeaderData(groupModel->fieldIndex("activity_manage"), Qt::Horizontal,"活动管理权");
    groupModel->setHeaderData(groupModel->fieldIndex("send_message"), Qt::Horizontal,"信息收发权限");
    groupModel->select();

    departmentModel->setTable("magic_department");
    departmentModel->setSort(departmentModel->fieldIndex("dpt_id"), Qt::AscendingOrder);
    departmentModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    departmentModel->setHeaderData(departmentModel->fieldIndex("dpt_id"),Qt::Horizontal,"编号");
    departmentModel->setHeaderData(departmentModel->fieldIndex("dpt_name"),Qt::Horizontal,"部门名称");
    departmentModel->select();

    isFirst = false;
    emit groupManageWorkFinished();
}

void GroupManageWork::setDB(const QSqlDatabase &db)
{
    DB = db;
}

void GroupManageWork::setGroupModel(QSqlTableModel *group)
{
    groupModel = group;
}

void GroupManageWork::setDepartmentModel(QSqlTableModel *department)
{
    departmentModel = department;
}

void GroupManageWork::submitAll(int type)
{
    if(type == 1)
        emit submitFinished_1(groupModel->submitAll());
    else
        emit submitFinished_0(departmentModel->submitAll());
}

void GroupManageWork::fixUser(int type, const QString &removedId)
{
    QString id = removedId;
    QSqlQuery query(DB);
    if(type ==1)
        query.exec("UPDATE magic_users SET user_group='2' WHERE user_group='" + id + "';");   //将已经删除的用户组恢复至默认普通用户
    else
        query.exec("UPDATE magic_users SET user_dpt='1' WHERE user_dpt='" + id + "';");//将已经删除的部门恢复至默认部门
    query.clear();
}
