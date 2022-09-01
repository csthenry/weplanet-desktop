#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include "service.h"

class ActivityManageWork : public QObject
{
	Q_OBJECT

public:
	ActivityManageWork(QObject *parent = nullptr);
	~ActivityManageWork();
	void working();
	void updateActStatus();
	void homeWorking();

	QSqlDatabase getDB();
	void submitAll();
	void setUid(QString uid);
	void setModel(QSqlTableModel* model);
	void setMemberModel(QSqlTableModel* model);
	void apply(const QString aid, const QString& uid);
	void cancel(const QString aid, const QString& uid);
	void delActivity(const QString aid);
	void m_approveAll(const QString aid);
	void m_approve(const QString actm_id);
	void m_reject(const QString actm_id);
	void m_delete(const QString actm_id);
	void setType(int t);
	float getCurScore();

private:
	float curScore = 0;
	int type;	//1为活动页面，2为活动管理页面
	QString uid;	//用于统计当前用户已完成活动
	service db_service;
	QSqlDatabase DB, DB_SECOND;
	QSqlTableModel* tabModel, *memberTabModel;
signals:
	void activityManageWorkFinished(int type);
	void actHomeWorkFinished();
	void submitAllFinished(bool);
	void operateFinished(QString error);
	void manageOperateFinished(QString error);
};
