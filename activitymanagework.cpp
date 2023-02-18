#include "activitymanagework.h"

ActivityManageWork::ActivityManageWork(QObject *parent)
	: QObject(parent)
{
    db_service.addDatabase(DB, "ActivityManageWork_DB");
    db_service.addDatabase(DB_SECOND, "ActivityManageWork_DB_SECOND");

    DB.setConnectOptions("MYSQL_OPT_RECONNECT=1");  //超时重连
    DB.open();
}

ActivityManageWork::~ActivityManageWork()
{
    DB.close();
}

void ActivityManageWork::working()
{
    tabModel->setTable("magic_activity");
    tabModel->setSort(tabModel->fieldIndex("joinDate"), Qt::DescendingOrder);
    tabModel->setEditStrategy(QSqlTableModel::OnRowChange);
    tabModel->setHeaderData(tabModel->fieldIndex("act_id"), Qt::Horizontal, "编号");
    tabModel->setHeaderData(tabModel->fieldIndex("act_name"), Qt::Horizontal, "活动名称");
    tabModel->setHeaderData(tabModel->fieldIndex("act_des"), Qt::Horizontal, "活动描述");
    tabModel->setHeaderData(tabModel->fieldIndex("joinDate"), Qt::Horizontal, "报名时间");
    tabModel->setHeaderData(tabModel->fieldIndex("beginDate"), Qt::Horizontal, "开始时间");
    tabModel->setHeaderData(tabModel->fieldIndex("endDate"), Qt::Horizontal, "结束时间");
    tabModel->setHeaderData(tabModel->fieldIndex("editUid"), Qt::Horizontal, "发布者UID");
    tabModel->setHeaderData(tabModel->fieldIndex("act_score"), Qt::Horizontal, "活动学时");
    tabModel->select();

    memberTabModel->setTable("magic_activityMembers");
    memberTabModel->setSort(memberTabModel->fieldIndex("actm_joinDate"), Qt::DescendingOrder);
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("actm_id"), Qt::Horizontal, "报名号");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("act_id"), Qt::Horizontal, "活动编号");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("actm_uid"), Qt::Horizontal, "用户UID");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("actm_joinDate"), Qt::Horizontal, "报名时间");
    memberTabModel->setHeaderData(memberTabModel->fieldIndex("status"), Qt::Horizontal, "活动状态");
    memberTabModel->select();
    if (type == 1)
        updateActStatus();  //更新已报名活动状态并统计学时
    emit activityManageWorkFinished(type);
}

void ActivityManageWork::updateActStatus()
{
    QDateTime curDataTime = QDateTime::currentDateTime();
    QSqlRecord curRec, actRec;

    memberTabModel->setFilter("actm_uid = '" + uid + "' AND status = '已录取'");
    for (int i = 0; !memberTabModel->record(i).value(0).toString().isEmpty(); i++)
    {
        curRec = memberTabModel->record(i);
        tabModel->setFilter("act_id = '" + curRec.value("act_id").toString() + "'");
        actRec = tabModel->record(0);
        
        if (actRec.value("endDate").toDateTime() <= curDataTime)
        {
            memberTabModel->setData(memberTabModel->index(i, 4), "已完成");
            memberTabModel->submitAll();
            //自己发布的活动不加学时
            if(!memberTabModel->lastError().isValid() && actRec.value("editUid").toString() != uid)
            {
                curScore += actRec.value("act_score").toFloat();
                qDebug() << "正在统计[" + actRec.value("act_name").toString() + "]学时：" << actRec.value("act_score").toFloat();
            }
        }
        tabModel->setFilter("");
    }
    memberTabModel->setFilter("actm_uid=" + uid);

    qDebug() << "当前待新增的总学时:" + QString::number(curScore);
}

void ActivityManageWork::homeWorking()
{
    QDateTime m_curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime()); //获取网络时间
    QString curDateTime = m_curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    tabModel->clear();
    tabModel->setTable("magic_activity");
    tabModel->setSort(tabModel->fieldIndex("joinDate"), Qt::DescendingOrder);
    tabModel->setEditStrategy(QSqlTableModel::OnRowChange);
    tabModel->setHeaderData(tabModel->fieldIndex("act_id"), Qt::Horizontal, "活动编号");
    tabModel->setHeaderData(tabModel->fieldIndex("act_name"), Qt::Horizontal, "活动名称");
    tabModel->setHeaderData(tabModel->fieldIndex("act_des"), Qt::Horizontal, "活动描述");
    tabModel->setHeaderData(tabModel->fieldIndex("beginDate"), Qt::Horizontal, "开始时间");
    tabModel->setHeaderData(tabModel->fieldIndex("endDate"), Qt::Horizontal, "结束时间");
    tabModel->setHeaderData(tabModel->fieldIndex("editUid"), Qt::Horizontal, "发布者UID");
    tabModel->setHeaderData(tabModel->fieldIndex("act_score"), Qt::Horizontal, "活动学时");
    tabModel->select();
    tabModel->setFilter("joinDate <= '" + curDateTime + "' AND beginDate >= '" + curDateTime + "'");
    emit actHomeWorkFinished();
}

QSqlDatabase ActivityManageWork::getDB()
{
    return DB;
}

void ActivityManageWork::submitAll()
{
    bool res = true;
    if (tabModel->isDirty())
    {
        DB_SECOND.open();
        res = tabModel->submitAll();
        QSqlQuery statistics(DB_SECOND);

        //统计活动发布量
        statistics.exec("SELECT * FROM magic_statistics WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
        if (statistics.next())
            statistics.exec("UPDATE magic_statistics SET activity_cnt=activity_cnt+1 WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
        else
            statistics.exec("INSERT INTO magic_statistics (date, activity_cnt) VALUES ('" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "', 1)");
        statistics.clear();
        DB_SECOND.close();
    }
    emit submitAllFinished(res);
}

void ActivityManageWork::setUid(QString uid)
{
    this->uid = uid;
}

void ActivityManageWork::setModel(QSqlTableModel* model)
{
    tabModel = model;
}

void ActivityManageWork::setMemberModel(QSqlTableModel* model)
{
    memberTabModel = model;
}

void ActivityManageWork::apply(const QString aid, const QString& uid)
{
    DB_SECOND.open();
    QDateTime curDateTime;
    curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime()); //获取网络时间
    QSqlQuery query(DB_SECOND);
    query.exec("INSERT INTO magic_activityMembers (act_id, actm_uid, actm_joinDate, status) VALUES (" + aid + ", " + uid + ", '" + curDateTime.toString("yyyy-MM-dd hh:mm:ss") + "', '待审核')");
    QString res = query.lastError().text();
    query.clear();
    DB_SECOND.close();
    emit operateFinished(res);
}

void ActivityManageWork::cancel(const QString aid, const QString& uid)
{
    DB_SECOND.open();
    QSqlQuery query(DB_SECOND);
    query.exec("DELETE FROM magic_activityMembers WHERE act_id=" + aid + " AND actm_uid=" + uid);
    QString res = query.lastError().text();
    query.clear();
    DB_SECOND.close();
    emit operateFinished(res);
}

void ActivityManageWork::delActivity(const QString aid)
{
    DB_SECOND.open();
    QSqlQuery query(DB_SECOND);
    query.exec("DELETE FROM magic_activity WHERE act_id=" + aid);
    QString res = query.lastError().text();
    if(res.isEmpty())
    {
        query.exec("DELETE FROM magic_activityMembers WHERE act_id=" + aid);
        QString res = query.lastError().text();
    }
    query.clear();
    DB_SECOND.close();
    emit manageOperateFinished(res);
}

void ActivityManageWork::m_approveAll(const QString aid)
{
    DB_SECOND.open();
    QSqlQuery query(DB_SECOND);
    query.exec("UPDATE magic_activityMembers SET status='已录取' WHERE act_id=" + aid + " AND status='待审核'");
    QString res = query.lastError().text();
    query.clear();
    DB_SECOND.close();
    emit manageOperateFinished(res);
}

void ActivityManageWork::m_approve(const QString actm_id)
{
    DB_SECOND.open();
    QSqlQuery query(DB_SECOND);
    query.exec("UPDATE magic_activityMembers SET status='已录取' WHERE actm_id=" + actm_id);
    QString res = query.lastError().text();
    query.clear();
    DB_SECOND.close();
    emit manageOperateFinished(res);
}

void ActivityManageWork::m_reject(const QString actm_id)
{
    DB_SECOND.open();
    QSqlQuery query(DB_SECOND);
    query.exec("UPDATE magic_activityMembers SET status='未录取' WHERE actm_id=" + actm_id);
    QString res = query.lastError().text();
    query.clear();
    DB_SECOND.close();
    emit manageOperateFinished(res);
}

void ActivityManageWork::m_delete(const QString actm_id)
{
    DB_SECOND.open();
    QSqlQuery query(DB_SECOND);
    query.exec("DELETE FROM magic_activityMembers WHERE actm_id=" + actm_id);
    if (query.lastError().text().isEmpty())
        query.exec("DELETE FROM magic_activityMembers WHERE actm_id=" + actm_id);
    QString res = query.lastError().text();
    query.clear();
    DB_SECOND.close();
    emit manageOperateFinished(res);
}

void ActivityManageWork::setType(int t)
{
    type = t;
}

void ActivityManageWork::setFilter(int flag, const QString& filter)
{
    if (flag == 0)
        tabModel->setFilter(filter);
    else
        memberTabModel->setFilter(filter);
}

float ActivityManageWork::getCurScore()
{
    float tmp = curScore;
    curScore = 0;   //清空待添加的学时
    return tmp;
}
