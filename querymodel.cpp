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
    tabModel->setSort(tabModel->fieldIndex("group_id"), Qt::AscendingOrder);    //升序排列
    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);  //手动提交
    tabModel->setHeaderData(tabModel->fieldIndex("group_id"), Qt::Horizontal,"编号");
    tabModel->setHeaderData(tabModel->fieldIndex("group_name"), Qt::Horizontal,"用户组名称");
    tabModel->setHeaderData(tabModel->fieldIndex("users_manage"), Qt::Horizontal,"用户管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("attend_manage"), Qt::Horizontal,"考勤管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("apply_manage"), Qt::Horizontal,"审批权限");
    tabModel->setHeaderData(tabModel->fieldIndex("applyItem_manage"), Qt::Horizontal,"审批流程管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("group_manage"), Qt::Horizontal,"团体架构管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("activity_manage"), Qt::Horizontal,"活动管理权");
    tabModel->setHeaderData(tabModel->fieldIndex("send_message"), Qt::Horizontal,"信息收发权限");
    if(!tabModel->select())
        return nullptr;

    if(tabModel->lastError().isValid())
        return nullptr;
    return tabModel;
}

QSqlRelationalTableModel *queryModel::setActAttendPage_relationalTableModel()
{
    relTableModel = new QSqlRelationalTableModel(parent, db);

    relTableModel->setTable("magic_attendance");
    relTableModel->setSort(relTableModel->fieldIndex("today"), Qt::DescendingOrder);    //时间降序排列
    relTableModel->setEditStrategy(QSqlTableModel::OnRowChange);  //自动提交
    //relTableModel->setHeaderData(relTableModel->fieldIndex("num"), Qt::Horizontal,"编号");
    relTableModel->setHeaderData(relTableModel->fieldIndex("a_uid"), Qt::Horizontal,"账号（UID）");
    relTableModel->setHeaderData(relTableModel->fieldIndex("begin_date"), Qt::Horizontal,"签到时间");
    relTableModel->setHeaderData(relTableModel->fieldIndex("end_date"), Qt::Horizontal,"签退时间");
    relTableModel->setHeaderData(relTableModel->fieldIndex("today"), Qt::Horizontal,"考勤日期");
    relTableModel->setHeaderData(relTableModel->fieldIndex("isSupply"), Qt::Horizontal,"是否补签");
    relTableModel->setHeaderData(relTableModel->fieldIndex("operator"), Qt::Horizontal,"签到来源");

    //建立外键关联
    relTableModel->setRelation(relTableModel->fieldIndex("operator"), QSqlRelation("magic_users", "uid", "name"));

    if(!relTableModel->select())
        return nullptr;

    if(relTableModel->lastError().isValid())
        return nullptr;
    return relTableModel;
}

QSqlRelationalTableModel *queryModel::setActUserPage_relationalTableModel()
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

    if(!relTableModel->select())
        return nullptr;

    if(relTableModel->lastError().isValid())
        return nullptr;
    return relTableModel;
}
