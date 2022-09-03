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
	void setWorkType(int type);
	void poster_statistics();

	bool cache = false;		//新内容缓存
	int cacheRow = -1;		//缓存row

private:
	int workType = -1;	//1管理页面；2用户页面
	service db_service;
	QSqlDatabase DB, DB_SECOND;
	QSqlTableModel* tabModel, *manageModel;
signals:
	void contentsManageWorkFinished();
	void contentsWorkFinished();
	void submitAllFinished(bool);
	void operateFinished(QString error);
};

#endif //POSTERWORK_H