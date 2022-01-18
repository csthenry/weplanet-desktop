/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "querymodel.h"
#include <QtDebug>
#include "service.h"

queryModel::queryModel()
{

}

queryModel::queryModel(QSqlDatabase db, QObject* parent)
{
    this->db = db;
    this->parent = parent;
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
    tabModel->setSort(tabModel->fieldIndex("group_id"), Qt::AscendingOrder);    //升序排列
    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);  //手动提交
    tabModel->setHeaderData(tabModel->fieldIndex("group_id"), Qt::Horizontal,"编号");
    tabModel->setHeaderData(tabModel->fieldIndex("group_name"), Qt::Horizontal,"用户组名称");
    tabModel->setHeaderData(tabModel->fieldIndex("users_manage"), Qt::Horizontal,"用户管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("attend_manage"), Qt::Horizontal,"考勤管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("apply_manage"), Qt::Horizontal,"审批权限");
    tabModel->setHeaderData(tabModel->fieldIndex("applyItem_manage"), Qt::Horizontal,"审批流程管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("group_manage"), Qt::Horizontal,"组织架构管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("activity_manage"), Qt::Horizontal,"活动管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("send_message"), Qt::Horizontal,"信息收发权限");
    if(!tabModel->select())
        return nullptr;

    if(tabModel->lastError().isValid())
        return nullptr;
    return tabModel;
}

QSqlTableModel *queryModel::setActivityPage()
{
    tabModel = new QSqlTableModel(parent, db);
    tabModel->setTable("magic_activity");
    tabModel->setSort(tabModel->fieldIndex("act_id"), Qt::AscendingOrder);
    tabModel->setEditStrategy(QSqlTableModel::OnRowChange);     //自动提交
    tabModel->setHeaderData(tabModel->fieldIndex("act_id"), Qt::Horizontal,"编号");
    tabModel->setHeaderData(tabModel->fieldIndex("act_name"), Qt::Horizontal,"活动名称");
    tabModel->setHeaderData(tabModel->fieldIndex("act_des"), Qt::Horizontal,"活动描述");
    tabModel->setHeaderData(tabModel->fieldIndex("joinDate"), Qt::Horizontal,"报名时间");
    tabModel->setHeaderData(tabModel->fieldIndex("beginDate"), Qt::Horizontal,"开始时间");
    tabModel->setHeaderData(tabModel->fieldIndex("endDate"), Qt::Horizontal,"结束时间");
    tabModel->setHeaderData(tabModel->fieldIndex("editUid"), Qt::Horizontal,"发布者UID");
    if(!tabModel->select())
        return nullptr;

    if(tabModel->lastError().isValid())
        return nullptr;
    return tabModel;
}

QSqlRelationalTableModel* queryModel::setActAttendPage_relationalTableModel()
{
    relTableModel = new QSqlRelationalTableModel(parent, db);
    relTableModel->setTable("magic_attendance");
    relTableModel->setSort(relTableModel->fieldIndex("today"), Qt::DescendingOrder);    //时间降序排列
    relTableModel->setEditStrategy(QSqlTableModel::OnRowChange);  //自动提交
    relTableModel->setHeaderData(relTableModel->fieldIndex("num"), Qt::Horizontal,"编号");
    relTableModel->setHeaderData(relTableModel->fieldIndex("a_uid"), Qt::Horizontal,"账号（UID）");
    relTableModel->setHeaderData(relTableModel->fieldIndex("begin_date"), Qt::Horizontal,"签到时间");
    relTableModel->setHeaderData(relTableModel->fieldIndex("end_date"), Qt::Horizontal,"签退时间");
    relTableModel->setHeaderData(relTableModel->fieldIndex("today"), Qt::Horizontal,"考勤日期");
    relTableModel->setHeaderData(relTableModel->fieldIndex("isSupply"), Qt::Horizontal,"是否补签");
    relTableModel->setHeaderData(relTableModel->fieldIndex("operator"), Qt::Horizontal,"签到来源");

    //建立外键关联
    relTableModel->setRelation(relTableModel->fieldIndex("operator"), QSqlRelation("magic_users", "uid", "name"));
    return relTableModel;
}

QSqlRelationalTableModel* queryModel::setActUserPage_relationalTableModel()
{
    relTableModel = new QSqlRelationalTableModel(parent, db);
    relTableModel->setTable("magic_users");
    relTableModel->setSort(relTableModel->fieldIndex("uid"), Qt::AscendingOrder);    //升序排列
    relTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);     //手动提交

    relTableModel->setHeaderData(relTableModel->fieldIndex("uid"), Qt::Horizontal,"账号（UID）");
    relTableModel->setHeaderData(relTableModel->fieldIndex("name"), Qt::Horizontal,"姓名");
    relTableModel->setHeaderData(relTableModel->fieldIndex("gender"), Qt::Horizontal,"性别");
    relTableModel->setHeaderData(relTableModel->fieldIndex("telephone"), Qt::Horizontal,"手机号");
    relTableModel->setHeaderData(relTableModel->fieldIndex("mail"), Qt::Horizontal,"邮箱");
    relTableModel->setHeaderData(relTableModel->fieldIndex("user_group"), Qt::Horizontal,"用户组");
    relTableModel->setHeaderData(relTableModel->fieldIndex("user_dpt"), Qt::Horizontal,"所在部门");
    relTableModel->setHeaderData(relTableModel->fieldIndex("user_avatar"), Qt::Horizontal,"头像地址");

    //建立外键关联
    relTableModel->setRelation(relTableModel->fieldIndex("user_group"), QSqlRelation("magic_group", "group_id", "group_name"));
    relTableModel->setRelation(relTableModel->fieldIndex("user_dpt"), QSqlRelation("magic_department", "dpt_id", "dpt_name"));
    return relTableModel;
}

void queryModel::analyseWorkTime(int data[])
{
    int cnt = 0;
    QTime workTime(0, 0, 0, 0), beginTime, endTime;
    QSqlRecord curRecord;
    do{
        curRecord = relTableModel->record(cnt);
        if(!curRecord.value("begin_date").isNull() && !curRecord.value("end_date").isNull())
        {
            beginTime = QTime::fromString(curRecord.value("begin_date").toString(), "hh:mm:ss");
            endTime = QTime::fromString(curRecord.value("end_date").toString(), "hh:mm:ss");
            workTime = workTime.addSecs(beginTime.secsTo(endTime));
            if(workTime.hour() < 4)
                data[0]++;
            if(workTime.hour() >= 4 && workTime.hour() <= 6)
                data[1]++;
            if(workTime.hour() >= 6 && workTime.hour() <= 8)
                data[2]++;
            if(workTime.hour() > 8)
                data[3]++;
            workTime.setHMS(0, 0, 0, 0);
        }
        cnt ++;
    }while(!curRecord.value("begin_date").isNull());
}

QSqlTableModel *queryModel::getTableModel()
{
    return tabModel;
}

QSqlRelationalTableModel *queryModel::getrelTableModel()
{
    return relTableModel;
}
