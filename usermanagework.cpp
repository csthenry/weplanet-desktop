﻿#include "usermanagework.h"

UserManageWork::UserManageWork(QObject* parent) : QObject(parent)
{
    db_service.addDatabase(DB, "UserManageWork_DB");
    DB.setConnectOptions("MYSQL_OPT_RECONNECT=1");  //超时重连
}

UserManageWork::~UserManageWork()
{
    if (heartBeat != nullptr)
        heartBeat->deleteLater();
    if (DB.isOpen())
        DB.close();
}

void UserManageWork::working(QSqlRelationalTableModel* model)
{
    relTableModel = model;
    if(heartBeat == nullptr)
        heartBeat = new QTimer();
    connect(heartBeat, &QTimer::timeout, this, [=]() {
        if (isDisplay && relTableModel != nullptr)
            relTableModel->select();
        else
            if (DB.isOpen())
                DB.close();
        }, Qt::UniqueConnection);

    isDisplay = true;

    if(!DB.isOpen())
        DB.open();
    //使用relationalModel时，这数据库不能关闭，否则外键的映射就没办法操作了...早知道不用relationalModel了，数据库连接很难管理...  

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
    relTableModel->setHeaderData(relTableModel->fieldIndex("score"), Qt::Horizontal, "活动学时");
    relTableModel->setHeaderData(relTableModel->fieldIndex("last_login"), Qt::Horizontal, "最后登录");
    //建立外键关联
    relTableModel->setRelation(relTableModel->fieldIndex("user_group"), QSqlRelation("magic_group", "group_id", "group_name"));
    relTableModel->setRelation(relTableModel->fieldIndex("user_dpt"), QSqlRelation("magic_department", "dpt_id", "dpt_name"));
    relTableModel->select();
    while (relTableModel->canFetchMore())
        relTableModel->fetchMore();  //加载超过256的其余数据

    //获取用户组和部门
    getComboxItems();

    emit userManageWorkFinished();
}

void UserManageWork::setHeartBeat(bool flag)
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

void UserManageWork::getComboxItems()
{
    //获取用户组和部门
    QString dbErrMsg;
    if (!DB_SERVICE_SECOND.connectDb(dbErrMsg, "_SECOND"))
        qDebug() << "dbErrMsg" << dbErrMsg;
    {
        QSqlDatabase DB_SECOND = QSqlDatabase::database(DB_SERVICE_SECOND.connectionName);
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
    }
    DB_SERVICE_SECOND.disconnectDb();
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

//void UserManageWork::setModel(QSqlRelationalTableModel *model)
//{
//    relTableModel = model;
//}

void UserManageWork::submitAll()
{
    bool res = false;
    if (relTableModel != nullptr)
        res = relTableModel->submitAll();
    emit submitAllFinished(res);
}

void UserManageWork::setFilter(const QString& filter)
{
    if(relTableModel != nullptr)
        relTableModel->setFilter(filter);
}

void UserManageWork::loadAvatar()
{
    curPix = service::getAvatar(avatarUrl);
    service::setAvatarStyle(curPix);
    emit avatarFinished(curPix);
}

void UserManageWork::queryAccount(const QString& account)
{
    QSqlRecord res;
    QString dbErrMsg;
    if (!DB_SERVICE_SECOND.connectDb(dbErrMsg, "_SECOND"))
        qDebug() << "dbErrMsg" << dbErrMsg;
    {
        QSqlDatabase DB_SECOND = QSqlDatabase::database(DB_SERVICE_SECOND.connectionName);
        QSqlQuery query(DB_SECOND);
        query.exec("SELECT * FROM magic_users WHERE uid=" + account);
        query.next();
        res = query.record();
        query.exec("SELECT group_name FROM magic_group WHERE group_id=" + res.value("user_group").toString());
        query.next();
        res.setValue("user_group", query.value(0));
        query.exec("SELECT dpt_name FROM magic_department WHERE dpt_id=" + res.value("user_dpt").toString());
        query.next();
        res.setValue("user_dpt", query.value(0));
        query.clear();
    }
    DB_SERVICE_SECOND.disconnectDb();
    emit queryAccountFinished(res);
}

void UserManageWork::setCurAvatarUrl(const QString& url)
{
    avatarUrl = url;
}

/*
void UserManageWork::getComboxItems(QStringList &comboxItems_group, QStringList &comboxItems_department)
{
    comboxItems_group = this->comboxItems_group;
    comboxItems_department = this->comboxItems_department;
}
*/

QSqlDatabase UserManageWork::getDB()
{
    return DB;
}

void UserManageWork::setCombox(QComboBox* group, QComboBox* department)
{
    m_group = group;
    m_department = department;
}

void UserManageWork::getVerify(const QString& uid)
{
    bool res = false;
    this->uid = uid;
    QString dbErrMsg;
    if (!DB_SERVICE_SECOND.connectDb(dbErrMsg, "_SECOND"))
        qDebug() << "dbErrMsg" << dbErrMsg;
    {
        QSqlDatabase DB_SECOND = QSqlDatabase::database(DB_SERVICE_SECOND.connectionName);
        QSqlQuery query(DB_SECOND);
        res = query.exec("SELECT * FROM magic_verify WHERE v_uid = " + uid);
        if (query.next())
        {
            verifyTag = query.value("vid").toInt();
            verifyInfo = query.value("info").toString();

            query.exec("SELECT * FROM magic_verifyList WHERE v_id = " + QString::number(verifyTag));
            query.next();
            verifyType = query.value("verify_name").toString();
        }
        else
        {
            verifyTag = -1;
            verifyType = "";
            verifyInfo = "";
        }
        query.clear();
    }
    DB_SERVICE_SECOND.disconnectDb();
    emit getVerifyFinished(res);
}

void UserManageWork::updateVerify(int type, int verifyTag, const QString& info)
{
    bool res = false;
    QString dbErrMsg;
    if (!DB_SERVICE_SECOND.connectDb(dbErrMsg, "_SECOND"))
        qDebug() << "dbErrMsg" << dbErrMsg;
    {
        QSqlDatabase DB_SECOND = QSqlDatabase::database(DB_SERVICE_SECOND.connectionName);
        QSqlQuery query(DB_SECOND);
        if (type == 0)
            res = query.exec("DELETE FROM magic_verify WHERE v_uid = " + uid);
        else if (type == 1)
            res = query.exec("INSERT INTO magic_verify (v_uid, vid, info) VALUES (" + uid + ", " + QString::number(verifyTag) + ", '" + info + "')");
        else
            res = query.exec("UPDATE magic_verify SET vid = " + QString::number(verifyTag) + ", info = '" + info + "' WHERE v_uid = " + uid);
        query.clear();
    }
    DB_SERVICE_SECOND.disconnectDb();
    if (res)
        getVerify(this->uid);
    emit updateVerifyFinished(res);
}

QString UserManageWork::getVerifyInfo()
{
    return verifyInfo;
}

QString UserManageWork::getVerifyType()
{
    return verifyType;
}

int UserManageWork::getVerifyTag()
{
    return verifyTag;
}

QString UserManageWork::getUid()
{
    return uid;
}

QSqlRelationalTableModel* UserManageWork::getModel()
{
    return relTableModel;
}