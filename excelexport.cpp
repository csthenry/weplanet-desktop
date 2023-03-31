/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "excelexport.h"

ExcelExport::ExcelExport()
{
}

ExcelExport::~ExcelExport()
{
}

bool ExcelExport::WriteExcel(const QString &filepath, QStack<QSqlRecord>& dataStack)
{
    if (filepath.isEmpty())
        return false;

    QFile csvfile(filepath);
    csvfile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&csvfile);

    QStringList headData;
    headData << "考勤编号"
             << "用户账号"
             << "签到时间"
             << "签退时间"
             << "考勤日期"
             << "是否补签"
             << "签到来源";
    curDateTime = QDateTime::currentDateTime();

    //写入表头
    int headcnt = 0;
    foreach(auto headTitle, headData)
    {
        if (++headcnt < headData.count())
            out << headTitle << ",";
        else
            out << headTitle << "\n";
    }
    //写入考勤数据
    while(!dataStack.isEmpty())
    {
        QSqlRecord lineData = dataStack.pop();
        for(int cnt = 0; cnt < lineData.count(); cnt++)
        {
            QString data = lineData.value(cnt).toString();
            if (cnt + 1 < lineData.count())
				out << data << ",";
			else
				out << data << "\n";
        }
    }
    csvfile.flush();
    csvfile.close();
    return true;
}
