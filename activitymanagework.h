#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include "querymodel.h"
#include "service.h"

class ActivityManageWork : public QObject
{
	Q_OBJECT

public:
	ActivityManageWork(QObject *parent = nullptr);
	~ActivityManageWork();
	bool isFirst = true;
	void working();
	QSqlDatabase getDB();
	void submitAll();
	void setModel(QSqlTableModel* model);

private:
	service db_service;
	QSqlDatabase DB;
	QSqlTableModel* tabModel;
signals:
	void activityManageWorkFinished();
	void submitAllFinished(bool);
};
