/***************************************************/
/* Copyright (c) 2017-2023 bytecho.net             */
/* Written by Henry                                */
/***************************************************/
#pragma once
#pragma execution_character_set("utf-8")

#ifndef EXCELEXPORT_H
#define EXCELEXPORT_H

#include <QSqlRecord>
#include <QTextstream>
#include <QDateTime>
#include <QFileDialog>
#include <QStack>
#include <QFile>
#include <QDir>

class ExcelExport {

public:
    ExcelExport();
    ~ExcelExport();
    bool WriteExcel(const QString& filepath, QStack<QSqlRecord>& dataStack);

private:
    QDateTime curDateTime;

signals:
    void exportExcelFinished(bool res);
};

#endif
