#include "approvalwork.h"

ApprovalWork::ApprovalWork(QObject *parent)
	: QObject(parent)
{
	db_service.addDatabase(DB, "ApprovalWork_DB");
}

void ApprovalWork::getManagePageApplyItems(const QString& uid)
{
	DB.open();
	QSqlQuery query(DB);
	query.exec(QString("SELECT * FROM magic_applyItems WHERE publisher=1 OR publisher=%1").arg(uid));
	QByteArray array;
	applyItems.clear();
	while (query.next()) {
		QByteArray array;
		QSqlRecord record = query.record();
		QDataStream stream(&array, QIODevice::WriteOnly);
		stream << record.value("item_id").toString() << record.value("title").toString() << record.value("options").toString() << record.value("publisher").toString() << record.value("auditor_list").toString() << record.value("isHide").toString();
		applyItems.push_back(array);
		
		//获取审核人姓名
		QStringList auditor_list = record.value("auditor_list").toString().split(";", QString::SkipEmptyParts);
		for (auto uid : auditor_list)
		{
			QSqlQuery query(DB);
			query.exec(QString("SELECT name FROM magic_users WHERE uid=%1").arg(uid));
			if (query.next())
				auditorName[uid] = query.value("name").toString();
			query.clear();
		}
	}
	query.clear();
	DB.close();
	
	getManagePageAuditorList();
	emit getManagePageApplyItemsFinished();
}

void ApprovalWork::getManagePageAuditorList()
{
	DB.open();
	QSqlQuery query(DB);
	QList<QString> groups;
	query.exec("SELECT * FROM magic_group WHERE apply_manage=1;");
	while (query.next())
	{
		QSqlRecord record = query.record();
		groups.push_back(record.value("group_id").toString());	//有审批权限的用户组
	}
	query.exec("SELECT * FROM magic_users;");
	auditorList.clear();
	while (query.next())
	{
		if (groups.indexOf(query.value("user_group").toString()) != -1)
		{
			auditorList.push_back(query.value("uid").toString());
			auditorName[query.value("uid").toString()] = query.value("name").toString();
		}
	}
	query.clear();
	DB.close();
}

QList<QByteArray> ApprovalWork::getApplyItems()
{
	return applyItems;
}

QList<QString> ApprovalWork::getAuditorList()
{
	return auditorList;
}

QString ApprovalWork::getAuditorName(const QString& uid)
{
	return auditorName[uid];
}

ApprovalWork::~ApprovalWork()
{}
