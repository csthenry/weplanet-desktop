#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include "service.h"

class ApprovalWork  : public QObject
{
	Q_OBJECT

private:
	service db_service;
	QSqlDatabase DB;
	QList<QByteArray> applyItems;
	QHash<QString, QString> auditorName;
	QList<QString> auditorList;
	
public:
	void getManagePageApplyItems(const QString& uid);
	void getManagePageAuditorList();	//获取具有审核权限的人员列表
	QList<QByteArray> getApplyItems();
	QList<QString> getAuditorList();
	QString getAuditorName(const QString& uid);
	
private slots:

signals:
	void getManagePageApplyItemsFinished();

public:
	ApprovalWork(QObject *parent = nullptr);
	~ApprovalWork();
};
