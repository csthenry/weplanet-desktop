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
            if(!memberTabModel->lastError().isValid())
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
    DB.open();
    QDateTime m_curDateTime = QDateTime::currentDateTime();
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
    tabModel->setFilter("beginDate <='" + curDateTime + "' AND endDate >='" + curDateTime + "'");
    emit actHomeWorkFinished();
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
    QDateTime curDateTime;
    curDateTime = QDateTime::currentDateTime();
    QSqlQuery query(DB);
    query.exec("INSERT INTO magic_activityMembers (act_id, actm_uid, actm_joinDate, status) VALUES (" + aid + ", " + uid + ", '" + curDateTime.toString("yyyy-MM-dd hh:mm:ss") + "', '待审核')");
    QString res = query.lastError().text();
    query.clear();
    emit operateFinished(res);
}

void ActivityManageWork::cancel(const QString aid, const QString& uid)
{
    QSqlQuery query(DB);
    query.exec("DELETE FROM magic_activityMembers WHERE act_id=" + aid + " AND actm_uid=" + uid);
    QString res = query.lastError().text();
    query.clear();
    emit operateFinished(res);
}

void ActivityManageWork::delActivity(const QString aid)
{
    QSqlQuery query(DB);
    query.exec("DELETE FROM magic_activity WHERE act_id=" + aid);
    QString res = query.lastError().text();
    if(res.isEmpty())
    {
        query.exec("DELETE FROM magic_activityMembers WHERE act_id=" + aid);
        QString res = query.lastError().text();
    }
    query.clear();
    emit manageOperateFinished(res);
}

void ActivityManageWork::m_approve(const QString actm_id)
{
    QSqlQuery query(DB);
    query.exec("UPDATE magic_activityMembers SET status='已录取' WHERE actm_id=" + actm_id);
    QString res = query.lastError().text();
    query.clear();
    emit manageOperateFinished(res);
}

void ActivityManageWork::m_reject(const QString actm_id)
{
    QSqlQuery query(DB);
    query.exec("UPDATE magic_activityMembers SET status='未录取' WHERE actm_id=" + actm_id);
    QString res = query.lastError().text();
    query.clear();
    emit manageOperateFinished(res);
}

void ActivityManageWork::m_delete(const QString actm_id)
{
    QSqlQuery query(DB);
    query.exec("DELETE FROM magic_activityMembers WHERE actm_id=" + actm_id);
    if (query.lastError().text().isEmpty())
        query.exec("DELETE FROM magic_activityMembers WHERE actm_id=" + actm_id);
    QString res = query.lastError().text();
    query.clear();
    emit manageOperateFinished(res);
}

void ActivityManageWork::setType(int t)
{
    type = t;
}

float ActivityManageWork::getCurScore()
{
    float tmp = curScore;
    curScore = 0;   //清空待添加的学时
    return tmp;
}
