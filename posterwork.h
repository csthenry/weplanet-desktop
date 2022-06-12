#pragma once
#pragma execution_character_set("utf-8")

#ifndef POSTERWORK_H
#define POSTERWORK_H

#include <QObject>
#include "service.h"

class PosterWork  : public QObject
{
	Q_OBJECT

public:
	PosterWork(QObject* parent = nullptr);
	~PosterWork();
	void working();

	QSqlDatabase getDB();
	void submitAll();
	void setModel(QSqlTableModel* model);
	void setManageModel(QSqlTableModel* model);
	// void apply(const QString aid, const QString& uid);
	// void cancel(const QString aid, const QString& uid);
	// void delContent(const QString aid);
	void setWorkType(int type);

private:
	int workType = -1;	//1管理页面；2用户页面
	service db_service;
	QSqlDatabase DB;
	QSqlTableModel* tabModel, *manageModel;
signals:
	void contentsManageWorkFinished();
	void contentsWorkFinished();
	void submitAllFinished(bool);
	void operateFinished(QString error);
};

#endif //POSTERWORK_H