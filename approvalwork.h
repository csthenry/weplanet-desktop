#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include "service.h"

class ApprovalWork  : public QObject
{
	Q_OBJECT

private:
	service db_service;
	QSqlDatabase DB, DB_SECOND;
	QList<QByteArray> applyItems, applyForms;
	QHash<QString, QString> auditorName, applyItemTitle;
	QList<QString> auditorList;
	QString modifyItemID;
	QList<QByteArray> currentProcess;	//当前审批流程进度（已按流程顺序排序），包含result->result_text->operate_time
	QHash<QString, QList<QByteArray>> applyFormsProcess;	//包含当前用户所有申请表的审批进程
	QHash<QString, QByteArray> simpleApplyItems; //simpleApplyItems 仅包含表magic_applyItems中的title、options
	QHash<QString, QList<QString>> applyAuditorList;
	QHash<QString, QString> currentFormOptionsText;	//当前申请表单 用户填写的文本
	QList<QByteArray> applyFormList, applyFormListDone;	//等待审核的列表，已经审核的列表（apply_id, uid, item_id, options, operate_time）
	QList<QString> authApplyTokenResultList;	//包含申请表id，申请人，申请项目名，审核状态，申请时间
	
public:
	void getManagePageApplyItems(const QString& uid);
	void getUserPageApplyItems(const QString& uid);
	void getManagePageAuditorList();	//获取具有审核权限的人员列表
	void getAllApplyFormList(const QString& uid);	//获取审批队列供管理员审核
	void getApplyProcess(const QString& apply_id, const QString& item_id);
	void addOrModifyApplyItem(int type, QByteArray array);	//0新增或1修改申请项 ByteArray 标题->选项->发布者->流程->isHide
	QByteArray getSimpleApplyItems(const QString& item_id);
	QList<QByteArray> getApplyFormList();
	QList<QByteArray> getApplyFormListDone();
	QList<QByteArray> getApplyItems();
	QList<QByteArray> getApplyForms();
	QList<QString> getAuditorList();
	QString getAuditorName(const QString& uid);
	QString getApplyItemTitle(const QString& id);
	void getApplyToken(const QString& id);
	void setModifyItemID(const QString& id);
	void deleteOrSwitchApplyItem(int type, const QString& id);	//0删除或 1开放/暂停 申请项
	void agreeOrRejectApply(const QString& apply_id, const QString& auditor, const QString& result, const QString& text);
	void submitOrCancelApply(int type, const QString& apply_id, QByteArray array = QByteArray());	//1提交或0撤销申请
	void authApplyToken(const QString& token);
	QList<QByteArray> getCurrentApplyProcess(const QString& id);
	QString getCurrentFormOptionsText(const QString& id);	//当前申请表单的填写项的文本
	QList<QString> getAuthApplyTokenResultList();
	
private slots:

signals:
	void getManagePageApplyItemsFinished();
	void getUserPageApplyItemsFinished();
	void getApplyFormListFinished();
	void addOrModifyApplyItemFinished(bool res);
	void deleteOrSwitchApplyItemFinished(bool res);
	void submitOrCancelApplyFinished(bool res);
	void getApplyTokenFinished(QString token);
	void agreeOrRejectApplyFinished(bool res);
	void authApplyTokenFinished(bool res);

public:
	ApprovalWork(QObject *parent = nullptr);
	~ApprovalWork();
};
