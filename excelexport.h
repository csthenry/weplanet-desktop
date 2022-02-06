/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/
#pragma once
#pragma execution_character_set("utf-8")

#ifndef EXCELEXPORT_H
#define EXCELEXPORT_H

#include <QObject>
#include <QAxObject>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QDebug>


class ExcelExport {

public:
    ExcelExport(QObject* parent);
    ~ExcelExport();

    bool WriteExcel(const QString& filePath, QSqlTableModel* tableModel, const QString& uid, int type);

private:
    QObject *parent;
    QAxObject *excel;
    QDateTime curDateTime;
};

#endif
