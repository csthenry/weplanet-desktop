#include "attendwork.h"

AttendWork::AttendWork(QObject *parent) : QObject(parent)
{
    db_service.addDatabase(DB, "AttendWork_DB");
    db_service.addDatabase(DB_SECOND, "AttendWork_DB_SECOND");
}

void AttendWork::working()
{
    DB.open();

    relTableModel->setTable("magic_attendance");
    relTableModel->setSort(relTableModel->fieldIndex("today"), Qt::DescendingOrder);    //时间降序排列
    relTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);  //手动提交
    relTableModel->setHeaderData(relTableModel->fieldIndex("num"), Qt::Horizontal,"编号");
    relTableModel->setHeaderData(relTableModel->fieldIndex("a_uid"), Qt::Horizontal,"账号（UID）");
    relTableModel->setHeaderData(relTableModel->fieldIndex("begin_date"), Qt::Horizontal,"签到时间");
    relTableModel->setHeaderData(relTableModel->fieldIndex("end_date"), Qt::Horizontal,"签退时间");
    relTableModel->setHeaderData(relTableModel->fieldIndex("today"), Qt::Horizontal,"考勤日期");
    relTableModel->setHeaderData(relTableModel->fieldIndex("isSupply"), Qt::Horizontal,"是否补签");
    relTableModel->setHeaderData(relTableModel->fieldIndex("operator"), Qt::Horizontal,"签到来源");

    //建立外键关联
    relTableModel->setRelation(relTableModel->fieldIndex("operator"), QSqlRelation("magic_users", "uid", "name"));
    relTableModel->select();
    relTableModel->setFilter("a_uid='" + uid +"'");

    //将未签退的考勤项签退，签退时间23:59:59
    DB_SECOND.open();
    QDateTime curDateTime = QDateTime::currentDateTime();
    QSqlQuery query(DB_SECOND);
    query.exec("UPDATE magic_attendance SET end_date='23:59:59' WHERE today<'" + curDateTime.date().toString("yyyy-MM-dd") + "' AND end_date IS NULL");
    query.clear();
    DB_SECOND.close();

    //分析工作时间
    analyseWorkTime();

    emit attendWorkFinished();
}

void AttendWork::homeChartWorking()
{
    DB.open();
    relTableModel->setTable("magic_attendance");
    relTableModel->setSort(relTableModel->fieldIndex("today"), Qt::DescendingOrder);    //时间降序排列
    relTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);  //手动提交
    relTableModel->select();
    relTableModel->setFilter("a_uid='" + uid + "'");
    analyseWorkTime();
    
    QJsonObject seriesObj;
    QJsonArray dateArray;

    seriesObj.insert("data_yStatus", weekAllWorkStatus);
    seriesObj.insert("data_yTime", weekMyWorkTime);
    seriesObj.insert("data_yMem", weekWorkMem);
    
    QString date;
    QDateTime curDateTime = QDateTime::currentDateTime();
    curDateTime = curDateTime.addDays(-7);
    for (int i = 7; i >= 1; i--)
    {
        curDateTime = curDateTime.addDays(1);
        date = curDateTime.date().toString("MM.dd");
        dateArray.append(date);
    }
    seriesObj.insert("data_x", dateArray);
    QString jsCode = QString("init(%1, 1)").arg(QString(QJsonDocument(seriesObj).toJson()));
    
    emit homeChartDone(jsCode);
    DB.close();
}

void AttendWork::analyseWorkTime()
{
    int cnt = 0;
    QTime workTime(0, 0, 0, 0), beginTime, endTime;
    QDateTime today = QDateTime::currentDateTime();
    QJsonArray weekMyWorkTime = { 0, 0, 0, 0, 0, 0, 0 }, weekWorkMem = { 0, 0, 0, 0, 0, 0, 0 };
    QJsonArray weekAllWorkStatus = { "未签到", "未签到" , "未签到" , "未签到" , "未签到" , "未签到" ,"未签到" };
    QSqlRecord curRecord;
    for(int i = 0; i < 4; i++)
        workTimeData[i] = 0;
    do{
        curRecord = relTableModel->record(cnt);
        if(!curRecord.value("begin_date").isNull() && !curRecord.value("end_date").isNull())
        {
            beginTime = QTime::fromString(curRecord.value("begin_date").toString(), "hh:mm:ss");
            endTime = QTime::fromString(curRecord.value("end_date").toString(), "hh:mm:ss");
            workTime = workTime.addSecs(beginTime.secsTo(endTime));
            if(workTime.hour() < 4)
                workTimeData[0]++;
            if(workTime.hour() >= 4 && workTime.hour() <= 6)
                workTimeData[1]++;
            if(workTime.hour() >= 6 && workTime.hour() <= 8)
                workTimeData[2]++;
            if(workTime.hour() > 8)
                workTimeData[3]++;
            workTime.setHMS(0, 0, 0, 0);
        }
        cnt ++;
    }while(!curRecord.value("begin_date").isNull());
    today = today.addDays(-7);
    for (int i = 0; i < 7; i++)
    {
        cnt = 0;
        today = today.addDays(1);
        workTime.setHMS(0, 0, 0);
        do{
            curRecord = relTableModel->record(cnt);
            if (today.date().toString("yyyy-MM-dd") == curRecord.value("today").toDateTime().date().toString("yyyy-MM-dd"))
            {
                beginTime = QTime::fromString(curRecord.value("begin_date").toString(), "hh:mm:ss");
                if (curRecord.value("end_date").isNull())
                    endTime = QTime::fromString(QDateTime::currentDateTime().time().toString(), "hh:mm:ss");
                else
                    endTime = QTime::fromString(curRecord.value("end_date").toString(), "hh:mm:ss");
                workTime = workTime.addSecs(beginTime.secsTo(endTime));
                weekAllWorkStatus[i] = "已签到";
            }
            cnt++;
        } while (!curRecord.value("begin_date").isNull());
        weekMyWorkTime[i] = QString::asprintf("%.2f", workTime.hour() + workTime.minute() / 60.0);
    }
    this->weekMyWorkTime = weekMyWorkTime;
    this->weekAllWorkStatus = weekAllWorkStatus;
    QString preFilter = relTableModel->filter();
    today = today.addDays(-7);
    for (int i = 0; i < 7; i++)
    {
        today = today.addDays(1);
        relTableModel->setFilter("today='" + today.date().toString("yyyy-MM-dd") + "'");
        weekWorkMem[i] = relTableModel->rowCount();
    }
    relTableModel->setFilter(preFilter);
    this->weekWorkMem = weekWorkMem;
}

void AttendWork::analyseWorkStatus()
{
}

QSqlRecord AttendWork::getRecord(const int index)
{
    QSqlRecord record;
    record = relTableModel->record(index);
    return record;
}

void AttendWork::setUid(const QString &uid)
{
    this->uid = uid;
}

void AttendWork::setModel(QSqlRelationalTableModel *relTableModel)
{
    this->relTableModel = relTableModel;
}

int AttendWork::fieldIndex(const QString &field)
{
    return relTableModel->fieldIndex(field);
}

int *AttendWork::getWorkTime()
{
    return workTimeData;
}

QJsonArray AttendWork::getWeekMyWorkTime()
{
    return weekMyWorkTime;
}

QJsonArray AttendWork::getWeekAllWorkStatus()
{
    return weekAllWorkStatus;
}

QJsonArray AttendWork::getWeekWorkMem()
{
    return weekWorkMem;
}

QSqlDatabase AttendWork::getDB()
{
    return DB;
}

void AttendWork::submitAll(int type)
{
    QDateTime cur = QDateTime::currentDateTime();
    if(type == 1)
        emit attendDone(relTableModel->submitAll());
    else
    {
        DB_SECOND.open();
        QSqlQuery query(DB_SECOND);
        bool res = query.exec("UPDATE magic_attendance SET end_date = '" + cur.time().toString("hh:mm:ss") + "' WHERE a_uid = '" + uid +"' AND today = '" + cur.date().toString("yyyy-MM-dd") + "';");
        query.clear();
        DB_SECOND.close();
        emit attendOutDone(res);
    }
}
