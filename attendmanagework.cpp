#include "attendmanagework.h"

AttendManageWork::AttendManageWork(QObject *parent) : QObject(parent)
{
    db_service.addDatabase(DB, "AttendManageWork_DB");
    db_service.addDatabase(DB_SECOND, "AttendManageWork_DB_SECOND");

    DB.setConnectOptions("MYSQL_OPT_RECONNECT=1");  //超时重连
}

AttendManageWork::~AttendManageWork()
{
    if (heartBeat != nullptr)
        heartBeat->deleteLater();
    if (DB.isOpen())
        DB.close();
}

void AttendManageWork::working(QSqlRelationalTableModel* m_userModel, QSqlRelationalTableModel* m_attendModel)
{
    userModel = m_userModel;
    attendModel = m_attendModel;

    if (heartBeat == nullptr)
        heartBeat = new QTimer();
    connect(heartBeat, &QTimer::timeout, this, [=]() {
        if (isDisplay && userModel != nullptr && attendModel != nullptr)
        {
            userModel->select();
            attendModel->select();
        }
        else
            if (DB.isOpen())
                DB.close();
        }, Qt::UniqueConnection);

    if (!DB.isOpen())
        DB.open();

    //使用relationalModel时，这数据库不能关闭，否则外键的映射就没办法操作了...早知道不用relationalModel了，数据库连接很难管理...  
    isDisplay = true;
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
    userModel->setHeaderData(userModel->fieldIndex("last_login"), Qt::Horizontal, "最后登录");

    //建立外键关联
    userModel->setRelation(userModel->fieldIndex("user_group"), QSqlRelation("magic_group", "group_id", "group_name"));
    userModel->setRelation(userModel->fieldIndex("user_dpt"), QSqlRelation("magic_department", "dpt_id", "dpt_name"));
    userModel->select();
    while (userModel->canFetchMore())
        userModel->fetchMore();  //加载超过256的其余数据

    attendModel->setTable("magic_attendance");
    attendModel->setSort(attendModel->fieldIndex("today"), Qt::DescendingOrder);    //时间降序排列
    attendModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
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
    while (attendModel->canFetchMore())
		attendModel->fetchMore();  //加载超过256的其余数据

    //将未签退的考勤项签退，签退时间23:59:59
    DB_SECOND.open();
    QDateTime curDateTime = QDateTime::currentDateTime();
    QSqlQuery query(DB_SECOND);
    query.exec("UPDATE magic_attendance SET end_date='23:59:59' WHERE today<'" + curDateTime.date().toString("yyyy-MM-dd") + "' AND end_date IS NULL");
    query.clear();
    DB_SECOND.close();

    //获取用户组和部门
    getComboxItems();

    isFirst = false;
    emit attendManageWorkFinished();
}

void AttendManageWork::dataOperate(int type)
{
    QDateTime curDateTime = QDateTime::currentDateTime();
    switch (type)
    {
    case 1:curDateTime = curDateTime.addMonths(-1);
        break;
    case 2:curDateTime = curDateTime.addMonths(-3);
        break;
    default:
        break;
    }

    DB_SECOND.open();
    QSqlQuery query(DB_SECOND);
    if(type == 1 || type == 2)
        query.exec("DELETE FROM magic_attendance WHERE today<'" + curDateTime.date().toString("yyyy-MM-dd") + "'");
    else if(type == 3)
        query.exec("DELETE FROM magic_attendance");
    else
        emit dataOperateFinished(false);
    if(query.lastError().isValid())
        emit dataOperateFinished(false);
    else
        emit dataOperateFinished(true);
    query.clear();
    DB_SECOND.close();
}

void AttendManageWork::setFilter(int type, const QString& filter)
{
    if (userModel == nullptr || attendModel == nullptr)
        return;
    if (type == 0)
        userModel->setFilter(filter);
    else
        attendModel->setFilter(filter);
}

void AttendManageWork::setHeartBeat(bool flag)
{
    if (heartBeat == nullptr)
        return;
    if (flag)
    {
        if (!heartBeat->isActive())
            heartBeat->start(MYSQL_TIME_OUT);
    }
    else
    {
        if (heartBeat->isActive())
            heartBeat->stop();
    }
}

void AttendManageWork::getComboxItems()
{
    //获取用户组和部门
    DB_SECOND.open();
    QSqlQuery comboxGroup(DB_SECOND);
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
    DB_SECOND.close();

    //初始化数据过滤comBox
    for (int i = m_group->count() - 1; i >= 1; i--)
        m_group->removeItem(i);
    //m_group->addItem("所有用户组");
    m_group->addItems(comboxItems_group);

    for (int i = m_department->count() - 1; i >= 1; i--)
        m_department->removeItem(i);
    //m_department->addItem("所有部门");
    m_department->addItems(comboxItems_department);
}

void AttendManageWork::setCurAvatarUrl(const QString url)
{
    avatarUrl = url;
}

//void AttendManageWork::setUserModel(QSqlRelationalTableModel *relTableModel)
//{
//    userModel = relTableModel;
//}
//
//void AttendManageWork::setAttendModel(QSqlRelationalTableModel *relTableModel)
//{
//    attendModel = relTableModel;
//}

QSqlDatabase AttendManageWork::getDB()
{
    return DB;
}

QSqlRelationalTableModel* AttendManageWork::getUserModel()
{
    return userModel;
}

QSqlRelationalTableModel* AttendManageWork::getAttendModel()
{
    return attendModel;
}

void AttendManageWork::getComboxItems(QStringList& comboxItems_group, QStringList& comboxItems_department)
{
    comboxItems_group = this->comboxItems_group;
    comboxItems_department = this->comboxItems_department;
}

void AttendManageWork::submitAll(int type)
{
    if (attendModel == nullptr)
    {
        if (type == 1)
            emit submitAddFinished(false);
        else
            emit submitDelFinished(false);
        return;
    }
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

void AttendManageWork::setCombox(QComboBox *group, QComboBox *department)
{
    m_group = group;
    m_department = department;
}