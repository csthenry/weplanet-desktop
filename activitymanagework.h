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
	void setMemberModel(QSqlTableModel* model);
	void apply(QString& uid);
	void setType(int t);

private:
	int type;
	service db_service;
	QSqlDatabase DB;
	QSqlTableModel* tabModel, *memberTabModel;
signals:
	void activityManageWorkFinished(int type);
	void submitAllFinished(bool);
};
