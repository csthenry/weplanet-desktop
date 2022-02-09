#include "attendmanagework.h"

AttendManageWork::AttendManageWork(QObject *parent) : QObject(parent)
{
    db_service.addDatabase(DB, "AttendManageWork_DB");
}

void AttendManageWork::working()
{
    DB.open();
    if(!isFirst)
    {
        userModel->select();
        attendModel->select();
        getComboxItems();

        emit attendManageWorkFinished();
        return;
    }
    if (userModel->tableName() != "magic_users")
    {
		userModel->database().open();   //该model数据库和AttendManageWork中不一致
        userModel->setTable("magic_users");
        userModel->setSort(userModel->fieldIndex("uid"), Qt::AscendingOrder);    //升序排列
        userModel->setEditStrategy(QSqlTableModel::OnManualSubmit);     //手动提交
        
        userModel->setHeaderData(userModel->fieldIndex("uid"), Qt::Horizontal, "账号（UID）");
        userModel->setHeaderData(userModel->fieldIndex("name"), Qt::Horizontal, "姓名");
        userModel->setHeaderData(userModel->fieldIndex("gender"), Qt::Horizontal, "性别");
        userModel->setHeaderData(userModel->fieldIndex("telephone"), Qt::Horizontal, "手机号");
        userModel->setHeaderData(userModel->fieldIndex("mail"), Qt::Horizontal, "邮箱");
        userModel->setHeaderData(userModel->fieldIndex("user_group"), Qt::Horizontal, "用户组");
        userModel->setHeaderData(userModel->fieldIndex("user_dpt"), Qt::Horizontal, "所在部门");
        userModel->setHeaderData(userModel->fieldIndex("user_avatar"), Qt::Horizontal, "头像地址");
        
        //建立外键关联
        userModel->setRelation(userModel->fieldIndex("user_group"), QSqlRelation("magic_group", "group_id", "group_name"));
        userModel->setRelation(userModel->fieldIndex("user_dpt"), QSqlRelation("magic_department", "dpt_id", "dpt_name"));
        userModel->select();
    }
    else
        userModel->select();

    attendModel->setTable("magic_attendance");
    attendModel->setSort(attendModel->fieldIndex("today"), Qt::DescendingOrder);    //时间降序排列
    attendModel->setEditStrategy(QSqlTableModel::OnRowChange);  //自动提交
    attendModel->setHeaderData(attendModel->fieldIndex("num"), Qt::Horizontal,"编号");
    attendModel->setHeaderData(attendModel->fieldIndex("a_uid"), Qt::Horizontal,"账号（UID）");
    attendModel->setHeaderData(attendModel->fieldIndex("begin_date"), Qt::Horizontal,"签到时间");
    attendModel->setHeaderData(attendModel->fieldIndex("end_date"), Qt::Horizontal,"签退时间");
    attendModel->setHeaderData(attendModel->fieldIndex("today"), Qt::Horizontal,"考勤日期");
    attendModel->setHeaderData(attendModel->fieldIndex("isSupply"), Qt::Horizontal,"是否补签");
    attendModel->setHeaderData(attendModel->fieldIndex("operator"), Qt::Horizontal,"签到来源");

    //建立外键关联
    attendModel->setRelation(attendModel->fieldIndex("operator"), QSqlRelation("magic_users", "uid", "name"));
    attendModel->select();

    //获取用户组和部门
    getComboxItems();

    isFirst = false;
    emit attendManageWorkFinished();
}

void AttendManageWork::getComboxItems()
{
    //获取用户组和部门
    QSqlQuery comboxGroup(DB);
    comboxItems_group.clear();
    comboxGroup.exec("SELECT * FROM magic_group");
    while (comboxGroup.next())
        comboxItems_group << comboxGroup.value("group_name").toString();
    comboxGroup.clear();
    comboxItems_department.clear();
    comboxGroup.exec("SELECT * FROM magic_department");
    while (comboxGroup.next())
        comboxItems_department << comboxGroup.value("dpt_name").toString();
    comboxGroup.clear();
}

void AttendManageWork::setCurAvatarUrl(const QString url)
{
    avatarUrl = url;
}

void AttendManageWork::setUserModel(QSqlRelationalTableModel *relTableModel)
{
    userModel = relTableModel;
}

void AttendManageWork::setAttendModel(QSqlRelationalTableModel *relTableModel)
{
    attendModel = relTableModel;
}

QSqlDatabase AttendManageWork::getDB()
{
    return DB;
}

void AttendManageWork::getComboxItems(QStringList& comboxItems_group, QStringList& comboxItems_department)
{
    comboxItems_group = this->comboxItems_group;
    comboxItems_department = this->comboxItems_department;
}

void AttendManageWork::submitAll(int type)
{
    if(type == 1)
        emit submitAddFinished(attendModel->submitAll());
    else
        emit submitDelFinished(attendModel->submitAll());
}

void AttendManageWork::loadAvatar()
{
    curPix = service::getAvatar(avatarUrl);
    service::setAvatarStyle(curPix);
    emit avatarFinished(curPix);
}
