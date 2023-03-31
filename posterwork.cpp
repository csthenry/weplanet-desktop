#include "posterwork.h"

PosterWork::PosterWork(QObject *parent)
	: QObject(parent)
{
    db_service.addDatabase(DB, "PosterManageWork_DB");
    db_service.addDatabase(DB_SECOND, "PosterManageWork_DB_SECOND");

    //DB.setConnectOptions("MYSQL_OPT_RECONNECT=1");  //超时重连
    heartBeat = new QTimer(this);
    connect(heartBeat, &QTimer::timeout, this, [=]() {
        if (isDisplay)
        {
            if(workType == 1)
                manageModel->select();
			else if(workType == 2)
				tabModel->select();
        }
        else
            if (DB.isOpen())
                DB.close();
        });
    heartBeat->start(MYSQL_TIME_OUT);
}

PosterWork::~PosterWork()
{
    heartBeat->stop();
    if (DB.isOpen())
        DB.close();
}

void PosterWork::working()
{
    if (workType == -1)
        return;
    if (workType != 1)
    {
        if (!DB.isOpen())
            DB.open();
        isDisplay = true;
        tabModel->setTable("magic_contents");
        tabModel->setSort(tabModel->fieldIndex("c_id"), Qt::DescendingOrder);
        tabModel->setHeaderData(tabModel->fieldIndex("c_id"), Qt::Horizontal, "编号");
        tabModel->setHeaderData(tabModel->fieldIndex("title"), Qt::Horizontal, "标题");
        tabModel->setHeaderData(tabModel->fieldIndex("author_id"), Qt::Horizontal, "发布者");
        tabModel->setFilter("isHide = 0");  //仅显示未隐藏
        tabModel->select();
        while (tabModel->canFetchMore())
            tabModel->fetchMore();  //加载超过256的其余数据
        emit contentsWorkFinished();
    }
    else
    {
        if (!DB.isOpen())
            DB.open();
        isDisplay = true;
        manageModel->setTable("magic_contents");
        manageModel->setSort(manageModel->fieldIndex("c_id"), Qt::DescendingOrder);
        manageModel->setEditStrategy(QSqlTableModel::OnRowChange);
        manageModel->setHeaderData(manageModel->fieldIndex("c_id"), Qt::Horizontal, "编号");
        manageModel->setHeaderData(manageModel->fieldIndex("title"), Qt::Horizontal, "标题");
        manageModel->select();
        while (manageModel->canFetchMore())
            manageModel->fetchMore();  //加载超过256的其余数据
        emit contentsManageWorkFinished();
    }
}

QSqlDatabase PosterWork::getDB()
{
    return DB;
}

void PosterWork::submitAll()
{
    manageModel->submitAll();
}

void PosterWork::setModel(QSqlTableModel* model)
{
    tabModel = model;
}

void PosterWork::setManageModel(QSqlTableModel* model)
{
    manageModel = model;
}

void PosterWork::setWorkType(int type)
{
    workType = type;
}

void PosterWork::poster_statistics()
{
    DB_SECOND.open();
    QSqlQuery statistics(DB_SECOND);

    //统计动态发布量
    statistics.exec("SELECT * FROM magic_statistics WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    if (statistics.next())
        statistics.exec("UPDATE magic_statistics SET dynamics_cnt=dynamics_cnt+1 WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    else
        statistics.exec("INSERT INTO magic_statistics (date, dynamics_cnt) VALUES ('" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "', 1)");
    statistics.clear();
    DB_SECOND.close();
}

void PosterWork::setFilter(int type, const QString& filter)
{
    if (type == 0)
        tabModel->setFilter(filter);
    else
        manageModel->setFilter(filter);
}
