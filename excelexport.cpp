/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "excelexport.h"

ExcelExport::ExcelExport(QObject *parent)
{
    this->parent = parent;
}

ExcelExport::~ExcelExport()
{
}

// type 1 - 导出所有记录，type 2 - 导出当天所有记录，type 3 - 导出当前用户所有记录
bool ExcelExport::WriteExcel(const QString &filePath, QSqlTableModel *tableModel, const QString &uid, int type)
{
    if (filePath.isEmpty())
        return false;

    excel = new QAxObject(parent);
    excel->setControl("Excel.Application");
    excel->dynamicCall("SetVisible(bool Visible)", false);
    excel->setProperty("DisplayAlerts", false);

    QAxObject *workbooks = excel->querySubObject("WorkBooks");
    workbooks->dynamicCall("Add");
    QAxObject *workbook = excel->querySubObject("ActiveWorkBook");
    QAxObject *worksheets = workbook->querySubObject("Sheets");
    QAxObject *worksheet = worksheets->querySubObject("Item(int)", 1);

    QStringList headData;
    headData << "账号（UID）"
             << "签到时间"
             << "签退时间"
             << "考勤日期"
             << "是否补签"
             << "签到来源";
    QSqlRecord curRecord = tableModel->record(0); //获取记录
    curDateTime = QDateTime::currentDateTime();

    //写入表头
    int column = 1;
    foreach (auto cur, headData)
    {
        QAxObject *Range = worksheet->querySubObject("Cells(int, int)", 1, column++);
        Range->dynamicCall("SetValue(const QString &)", cur);
    }
    //写入考勤数据
    switch (type)
    {
    case 1:
        tableModel->setFilter("");
        for (int i = 1; curRecord.value("num").isValid(); i++)
        {
            curRecord = tableModel->record(i - 1);
            for (int j = 1; curRecord.value(j).isValid(); j++)
            {
                QAxObject *Range = worksheet->querySubObject("Cells(int, int)", i + 1, j); //从第二行开始
                Range->dynamicCall("SetValue(const QString &)", curRecord.value(j).toString());
            }
        }
        tableModel->setFilter("a_uid='" + uid +"'");
        break;
    case 2:
        tableModel->setFilter("");
        for (int i = 1; curRecord.value("num").isValid(); i++)
        {
            curRecord = tableModel->record(i - 1);
            if (curRecord.value("today") != curDateTime.date().toString("yyyy-MM-dd"))
                continue;
            for (int j = 1; curRecord.value(j).isValid(); j++)
            {
                QAxObject *Range = worksheet->querySubObject("Cells(int, int)", i + 1, j); //从第二行开始
                Range->dynamicCall("SetValue(const QString &)", curRecord.value(j).toString());
            }
        }
        tableModel->setFilter("a_uid='" + uid +"'");
        break;
    case 3:
        tableModel->setSort(tableModel->fieldIndex("a_uid"), Qt::DescendingOrder); //暂时按UID排列，避免出现空行
        for (int i = 1; curRecord.value("num").isValid(); i++)
        {
            curRecord = tableModel->record(i - 1);
            if (curRecord.value("a_uid") != uid)
                continue;
            for (int j = 1; curRecord.value(j).isValid(); j++)
            {
                QAxObject *Range = worksheet->querySubObject("Cells(int, int)", i + 1, j); //从第二行开始
                Range->dynamicCall("SetValue(const QString &)", curRecord.value(j).toString());
            }
        }
        tableModel->setSort(tableModel->fieldIndex("today"), Qt::DescendingOrder); //恢复时间降序排列
        break;
    default:
        return false;
    }
    workbook->dynamicCall("SaveAs(const QString &)", QDir::toNativeSeparators(filePath));
    if (excel != NULL)
    {
        excel->dynamicCall("Quit()");
        delete excel;
        return true;
    }
    else
        return false;
}
