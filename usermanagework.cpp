#include "usermanagework.h"

UserManageWork::UserManageWork(QObject *parent) : QObject(parent)
{
    db_service.addDatabase(DB, "UserManageWork_DB");
}

void UserManageWork::working()
{
    DB.open();  //使用model时，数据库应保持开启
    if(!isFirst)
    {
        relTableModel->select();
        getComboxItems();
        emit userManageWorkFinished();
        return;
    }
    if (relTableModel->tableName() != "magic_users")
    {
        relTableModel->setTable("magic_users");
        relTableModel->setSort(relTableModel->fieldIndex("uid"), Qt::AscendingOrder);    //升序排列
        relTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);     //手动提交

        relTableModel->setHeaderData(relTableModel->fieldIndex("uid"), Qt::Horizontal, "账号（UID）");
        relTableModel->setHeaderData(relTableModel->fieldIndex("name"), Qt::Horizontal, "姓名");
        relTableModel->setHeaderData(relTableModel->fieldIndex("gender"), Qt::Horizontal, "性别");
        relTableModel->setHeaderData(relTableModel->fieldIndex("telephone"), Qt::Horizontal, "手机号");
        relTableModel->setHeaderData(relTableModel->fieldIndex("mail"), Qt::Horizontal, "邮箱");
        relTableModel->setHeaderData(relTableModel->fieldIndex("user_group"), Qt::Horizontal, "用户组");
        relTableModel->setHeaderData(relTableModel->fieldIndex("user_dpt"), Qt::Horizontal, "所在部门");
        relTableModel->setHeaderData(relTableModel->fieldIndex("user_avatar"), Qt::Horizontal, "头像地址");

        //建立外键关联
        relTableModel->setRelation(relTableModel->fieldIndex("user_group"), QSqlRelation("magic_group", "group_id", "group_name"));
        relTableModel->setRelation(relTableModel->fieldIndex("user_dpt"), QSqlRelation("magic_department", "dpt_id", "dpt_name"));

        relTableModel->select();
    }
    else
        relTableModel->select();
    //获取用户组和部门
    getComboxItems();
    isFirst = false;
    emit userManageWorkFinished();
}

void UserManageWork::getComboxItems()
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

void UserManageWork::setModel(QSqlRelationalTableModel *model)
{
    relTableModel = model;
}

void UserManageWork::submitAll()
{
    bool res = true;
    if (relTableModel->isDirty())
        res = relTableModel->submitAll();
    emit submitAllFinished(res);
}

void UserManageWork::loadAvatar()
{
    curPix = service::getAvatar(avatarUrl);
    service::setAvatarStyle(curPix);
    emit avatarFinished(curPix);
}

void UserManageWork::setCurAvatarUrl(const QString &url)
{
    avatarUrl = url;
}

void UserManageWork::getComboxItems(QStringList &comboxItems_group, QStringList &comboxItems_department)
{
    comboxItems_group = this->comboxItems_group;
    comboxItems_department = this->comboxItems_department;
}

QSqlDatabase UserManageWork::getDB()
{
    return DB;
}
