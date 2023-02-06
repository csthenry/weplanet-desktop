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
	QString modifyItemID;
	
public:
	void getManagePageApplyItems(const QString& uid);
	void getManagePageAuditorList();	//获取具有审核权限的人员列表
	void addOrModifyApplyItem(int type, QByteArray array);	//0新增或1修改申请项 ByteArray标题->选项->发布者->流程->isHide
	QList<QByteArray> getApplyItems();
	QList<QString> getAuditorList();
	QString getAuditorName(const QString& uid);
	void setModifyItemID(const QString& id);
	void deleteOrSwitchApplyItem(int type, const QString& id);	//0删除或 1开放/暂停 申请项
	
private slots:

signals:
	void getManagePageApplyItemsFinished();
	void addOrModifyApplyItemFinished(bool res);
	void deleteOrSwitchApplyItemFinished(bool res);

public:
	ApprovalWork(QObject *parent = nullptr);
	~ApprovalWork();
};
