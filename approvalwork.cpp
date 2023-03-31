#include "approvalwork.h"

ApprovalWork::ApprovalWork(QObject *parent)
	: QObject(parent)
{
	db_service.addDatabase(DB, "ApprovalWork_DB");
	db_service.addDatabase(DB_SECOND, "ApprovalWork_DB_SECOND");
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

void ApprovalWork::getAllApplyFormList(const QString& uid)
{
	DB.open();
	QSqlQuery query(DB);

	applyFormList.clear();
	applyFormListDone.clear();

	query.exec("SELECT * FROM magic_applyItems");	//获取所有审批项目的流程
	while (query.next())
	{
		QByteArray array;
		QDataStream stream(&array, QIODevice::WriteOnly);
		QSqlRecord record = query.record();
		stream << record.value("title").toString() << record.value("options").toString();
		applyItemTitle.insert(record.value("item_id").toString(), record.value("title").toString());
		simpleApplyItems.insert(record.value("item_id").toString(), array);
		applyAuditorList.insert(record.value("item_id").toString(), record.value("auditor_list").toString().split(";", QString::SkipEmptyParts));
	}
	QList<QByteArray> allApplyFormList;
	query.exec("SELECT * FROM magic_apply ORDER BY apply_id DESC");	//获取所有申请表
	while (query.next())
	{
		if (applyAuditorList[query.value("item_id").toString()].indexOf(uid) != -1)	//需要我审核的申请表
		{
			QByteArray array;
			QDataStream stream(&array, QIODevice::WriteOnly);
			stream << query.value("apply_id").toString() << query.value("uid").toString() << query.value("item_id").toString() << query.value("options").toString() << query.value("operate_time").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
			allApplyFormList.push_back(array);
		}
	}
	for (auto applyForm : allApplyFormList)
	{
		QDataStream stream(&applyForm, QIODevice::ReadOnly);
		QString apply_id, m_uid, item_id, options, operate_time;
		stream >> apply_id >> m_uid >> item_id >> options >> operate_time;
		QStringList auditor_list = applyAuditorList[item_id];
		for (auto auditor: auditor_list)	//遍历审批流程，是否应该当前用户审核
		{
			query.exec(QString("SELECT * FROM magic_applyProcess WHERE apply_id = '%1' AND auditor = '%2'").arg(apply_id, auditor));
			bool isNext = query.next();
			if (isNext && query.value("result").toString() == "1")
			{
				//有审核记录，且通过
				if (auditor != uid)
					continue;
				else 
				{
					applyFormListDone.push_back(applyForm);	//已经被我审核
					break;
				}
			}
			else
			{
				//无审核记录，或已经拒绝
				QString res;
				if (isNext)
					res = query.value("result").toString();
				if (auditor == uid && res.isEmpty())
					applyFormList.push_back(applyForm);	//需要我审核
				else
				{
					if(auditor == uid && !res.isEmpty())
						applyFormListDone.push_back(applyForm);	//已经被我审核
					else
						break;	//还没到我审核，或流程已经终止
				}
			}
		}
		
	}
	
	query.clear();
	DB.close();

	emit getApplyFormListFinished();
}

void ApprovalWork::getUserPageApplyItems(const QString& uid)
{
	DB.open();
	QSqlQuery query(DB);
	//获取可申请的项目
	query.exec(QString("SELECT * FROM magic_applyItems WHERE isHide=0"));
	applyItems.clear();
	while (query.next()) {
		QByteArray array;
		QSqlRecord record = query.record();
		QDataStream stream(&array, QIODevice::WriteOnly);
		stream << record.value("item_id").toString() << record.value("title").toString() << record.value("options").toString() << record.value("publisher").toString() << record.value("auditor_list").toString() << record.value("isHide").toString();
		applyItems.push_back(array);
		applyItemTitle.insert(record.value("item_id").toString(), record.value("title").toString());
		applyAuditorList.insert(record.value("item_id").toString(), record.value("auditor_list").toString().split(";", QString::SkipEmptyParts));
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

	//获取用户已提交的申请
	applyForms.clear();
	query.exec(QString("SELECT * FROM magic_apply WHERE uid=%1 ORDER BY apply_id DESC").arg(uid));	//id降序
	DB_SECOND.open();	//调用getApplyProcess()前需打开该数据库
	while(query.next())
	{
		QByteArray array;
		QSqlRecord record = query.record();
		QDataStream stream(&array, QIODevice::WriteOnly);
		stream << record.value("apply_id").toString() << record.value("uid").toString() << record.value("item_id").toString() << record.value("options").toString() << record.value("status").toString() << record.value("token").toString() << record.value("operate_time").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
		applyForms.push_back(array);
		currentFormOptionsText.insert(record.value("apply_id").toString(), record.value("options").toString());
		int new_status = getApplyProcess(record.value("apply_id").toString(), record.value("item_id").toString());	//获取当前申请表审批进度
		if (QString::number(new_status) != record.value("status").toString())
		{
			//更新申请表状态
			QByteArray array;
			QDataStream stream(&array, QIODevice::WriteOnly);
			applyForms.pop_back();
			stream << record.value("apply_id").toString() << record.value("uid").toString() << record.value("item_id").toString() << record.value("options").toString() << QString::number(new_status) << record.value("token").toString() << record.value("operate_time").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
			applyForms.push_back(array);
		}
	}
	query.clear();
	DB_SECOND.close();	//关闭调用getApplyProcess()所需数据库连接
	DB.close();	
	emit getUserPageApplyItemsFinished();
}

void ApprovalWork::autoExecuteSystemApplyItems()
{
	DB.open();
	QSqlQuery query(DB);
	int step = 0, finishedNum = 0;	//已成功通过的流程，本次自动处理个数

	//处理系统申请项：个人信息异动申请
	QList<QString> apply_list;	//待自动处理的申请表
	query.exec(QString("SELECT * FROM magic_apply WHERE item_id = '1' AND (status = '0' OR status = '1')"));
	while(query.next())
		apply_list.push_back(query.value("apply_id").toString());
	for (auto apply_id : apply_list)
	{
		for (auto uid : applyAuditorList["1"])
		{
			QByteArray processOneStep;	//申请表流程中的一步
			query.exec(QString("SELECT * FROM magic_applyProcess WHERE apply_id = '%1' AND auditor = '%2'").arg(apply_id, uid));
			if (query.next())
			{
				if (query.value("result").toString() == "0")	//0为拒绝通过，流程终止
				{
					query.exec(QString("UPDATE magic_apply SET status=2 WHERE apply_id='%1'").arg(apply_id));	//将申请表状态改为已终止
					break;	//流程终止
				}
				step++;
			}
			else
				break;	//流程终止
		}
		if (step == applyAuditorList["1"].count())
		{
			query.exec(QString("UPDATE magic_apply SET status=1 WHERE apply_id='%1'").arg(apply_id));	//已通过
			//更新用户信息
			query.exec(QString("SELECT uid, options FROM magic_apply WHERE apply_id='%1' AND status=1").arg(apply_id));
			if (query.next())
			{
				QSqlRecord record = query.record();
				QList<QString> options = record.value("options").toString().split("$", QString::SkipEmptyParts);
				if (options.count() == 4)
				{
					DB_SECOND.open();
					QSqlQuery subQuery(DB_SECOND);
					if(options[0] != "无")	//更新姓名
						subQuery.exec(QString("UPDATE magic_users SET name='%1' WHERE uid=%2").arg(options[0], record.value("uid").toString()));
					if(options[1] != "无" && (options[1] == "男" || options[1] == "女"))	//更新性别
						subQuery.exec(QString("UPDATE magic_users SET gender='%1' WHERE uid=%2").arg(options[1], record.value("uid").toString()));
					if (options[2] != "无")	//更新手机号
						subQuery.exec(QString("UPDATE magic_users SET telephone='%1' WHERE uid=%2").arg(options[2], record.value("uid").toString()));
					subQuery.exec(QString("UPDATE magic_apply SET status=3 WHERE apply_id='%1'").arg(apply_id));	//已自动处理数据
					subQuery.exec(QString("SELECT mail FROM magic_users WHERE uid='%1';").arg(record.value("uid").toString()));
					QString mail;
					if (subQuery.next())
						mail = subQuery.value("mail").toString();
					DB_SECOND.close();
					finishedNum++; 
					if(!mail.isEmpty())
						service::sendMail(smtp_config, mail, "WePlanet 审批进度更新", QString("用户%1：\n你的【个人信息异动申请】现已完成审核，个人信息已更新。\n\n注：若异动信息有误，请联系管理员。").arg(record.value("uid").toString()));
				}
			}
		}
	}

	//处理系统申请项：账号认证申请
	step = 0;
	apply_list.clear();
	query.exec(QString("SELECT * FROM magic_apply WHERE item_id = '2' AND (status = '0' OR status = '1')"));
	while (query.next())
		apply_list.push_back(query.value("apply_id").toString());
	for (auto apply_id : apply_list)
	{
		for (auto uid : applyAuditorList["2"])
		{
			QByteArray processOneStep;	//申请表流程中的一步
			query.exec(QString("SELECT * FROM magic_applyProcess WHERE apply_id = '%1' AND auditor = '%2'").arg(apply_id, uid));
			if (query.next())
			{
				if (query.value("result").toString() == "0")	//0为拒绝通过，流程终止
				{
					query.exec(QString("UPDATE magic_apply SET status=2 WHERE apply_id='%1'").arg(apply_id));	//将申请表状态改为已终止
					break;	//流程终止
				}
				step++;
			}
			else
				break;	//流程终止
		}
		if (step == applyAuditorList["2"].count())
		{
			query.exec(QString("UPDATE magic_apply SET status=1 WHERE apply_id='%1'").arg(apply_id));	//已通过
			//更新用户认证信息
			query.exec(QString("SELECT uid, options FROM magic_apply WHERE apply_id='%1' AND status=1").arg(apply_id));
			while (query.next())
			{
				QSqlRecord record = query.record();
				QList<QString> options = record.value("options").toString().split("$", QString::SkipEmptyParts);
				if (options.count() == 3 && (options[0] == "1" || options[0] == "2"))
				{
					DB_SECOND.open();
					QSqlQuery subQuery(DB_SECOND);
					subQuery.exec(QString("INSERT INTO magic_verify (v_uid, vid, info) VALUES ('%1', '%2', '%3') ON DUPLICATE KEY UPDATE vid='%4', info='%5';").arg(record.value("uid").toString(), options[0], options[1], options[0], options[1]));
					subQuery.exec(QString("UPDATE magic_apply SET status=3 WHERE apply_id='%1'").arg(apply_id));	//已自动处理数据
					subQuery.exec(QString("SELECT mail FROM magic_users WHERE uid='%1';").arg(record.value("uid").toString()));
					QString mail;
					if (subQuery.next())
						mail = subQuery.value("mail").toString();
					DB_SECOND.close();
					finishedNum++;
					if (!mail.isEmpty())
						service::sendMail(smtp_config, mail, "WePlanet 审批进度更新", QString("用户%1：\n你的【账号认证申请】现已完成审核，账号认证信息已更新。\n\n注：若认证信息有误，请联系管理员。").arg(record.value("uid").toString()));
				}
			}
		}
	}

	query.clear();
	DB.close();

	emit autoExecuteSystemApplyItemsFinished(finishedNum);
}

int ApprovalWork::getApplyProcess(const QString& apply_id, const QString& item_id)	//调用此函数前需打开数据库DB_SECOND连接
{
	//DB_SECOND.open();
	QSqlQuery query(DB_SECOND);
	currentProcess.clear();
	int step = 0, apply_status = 0;	//已成功通过的流程，申请表状态
	for (auto uid : applyAuditorList[item_id])
	{
		QByteArray processOneStep;	//申请表流程中的一步
		query.exec(QString("SELECT * FROM magic_applyProcess WHERE apply_id = '%1' AND auditor = '%2'").arg(apply_id, uid));
		if(query.next())
		{
			QSqlRecord record = query.record();
			QDataStream stream(&processOneStep, QIODevice::WriteOnly);
			stream << record.value("result").toString() << record.value("result_text").toString() << record.value("operate_time").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
			currentProcess.push_back(processOneStep);
			if (record.value("result").toString() == "0")	//0为拒绝通过，流程终止
			{
				query.exec(QString("UPDATE magic_apply SET status=2 WHERE apply_id='%1'").arg(apply_id));	//将申请表状态改为已终止
				apply_status = 2;
				break;	//流程终止
			}
			step++;
		}
		else
			break;	//流程终止
	}
	if (step == applyAuditorList[item_id].count())
	{
		query.exec(QString("SELECT status FROM magic_apply WHERE apply_id='%1'").arg(apply_id));
		query.next();
		if(query.value("status").toString() != "3")		//已自动化处理的表单不更新状态
			query.exec(QString("UPDATE magic_apply SET status=1 WHERE apply_id='%1'").arg(apply_id));	//已通过
		apply_status = 1;
	}
	applyFormsProcess.insert(apply_id, currentProcess);	//将所有已审核的步骤存入对应id键值
	
	query.clear();
	//DB_SECOND.close();
	return apply_status;
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

void ApprovalWork::addOrModifyApplyItem(int type, QByteArray array)
{
	QDataStream stream(&array, QIODevice::ReadOnly);
	QString title, options, auditorList, publisher, isHide;
	stream >> title >> options >> publisher >> auditorList >> isHide;
	DB.open();
	QSqlQuery query(DB);
	bool res = false;
	if (type == 0)
		res = query.exec(QString("INSERT INTO magic_applyItems (title, options, publisher, auditor_list, isHide) VALUES ('%1', '%2', '%3', '%4', '%5')").arg(title, options, publisher, auditorList, isHide));
	else
		res = query.exec(QString("UPDATE magic_applyItems SET options='%1', auditor_list='%2' WHERE item_id='%3'").arg(options, auditorList, modifyItemID));

	query.clear();
	DB.close();
	emit addOrModifyApplyItemFinished(res);
}

void ApprovalWork::getApplyToken(const QString& id)
{
	DB.open();
	QSqlQuery query(DB);
	QString token;
	bool res = false;
	res = query.exec(QString("SELECT token FROM magic_apply WHERE apply_id = '%1'").arg(id));

	if (query.next() && query.value("token").toString() != "")
		token = query.value("token").toString();
	else
	{
		QString dateTime = QString::number(service::getWebTime());
		token = QCryptographicHash::hash((id + "_" + dateTime).toUtf8(), QCryptographicHash::Md5).toHex();
		res = query.exec(QString("UPDATE magic_apply SET token = '%1' WHERE apply_id = '%2'").arg(token, id));
	}
	query.clear();
	DB.close();
	if (!res)
		token = "error";
	emit getApplyTokenFinished(token);
}

QList<QByteArray> ApprovalWork::getApplyItems()
{
	return applyItems;
}

QList<QByteArray> ApprovalWork::getApplyForms()
{
	return applyForms;
}

QByteArray ApprovalWork::getSimpleApplyItems(const QString& item_id)
{
	return simpleApplyItems[item_id];
}

QList<QByteArray> ApprovalWork::getApplyFormList()
{
	return applyFormList;
}

QList<QByteArray> ApprovalWork::getApplyFormListDone()
{
	return applyFormListDone;
}

QList<QString> ApprovalWork::getAuditorList()
{
	return auditorList;
}

QString ApprovalWork::getAuditorName(const QString& uid)
{
	return auditorName[uid];
}

QString ApprovalWork::getApplyItemTitle(const QString& id)
{
	return applyItemTitle[id];
}

void ApprovalWork::setModifyItemID(const QString& id)
{
	modifyItemID = id;
}

void ApprovalWork::deleteOrSwitchApplyItem(int type, const QString& id)
{
	DB.open();
	QSqlQuery query(DB);
	bool res = false;
	if(type == 0)
		res = query.exec("DELETE FROM magic_applyItems WHERE item_id=" + id);
	else
	{
		query.exec("SELECT isHide FROM magic_applyItems WHERE item_id=" + id);
		query.next();
		if (query.value("isHide").toString() == "0")
			res = query.exec("UPDATE magic_applyItems SET isHide=1 WHERE item_id=" + id);
		else
			res = query.exec("UPDATE magic_applyItems SET isHide=0 WHERE item_id=" + id);
	}
	query.clear();
	DB.close();

	emit deleteOrSwitchApplyItemFinished(res);
}

void ApprovalWork::agreeOrRejectApply(const QString& apply_id, const QString& auditor, const QString& result, const QString& text)
{
	DB.open();
	QSqlQuery query(DB);
	bool res = query.exec(QString("INSERT INTO magic_applyProcess (apply_id, auditor, result, result_text, operate_time) VALUES('%1', '%2', '%3', '%4', NOW())").arg(apply_id, auditor, result, text));

	emit agreeOrRejectApplyFinished(res);
}

void ApprovalWork::submitOrCancelApply(int type, const QString& apply_id, QByteArray array)
{
	DB.open();
	QSqlQuery query(DB);
	bool res = false;
	if (type == 1)
	{
		QString uid, item_id, options, operate_time;
		QDataStream stream(&array, QIODevice::ReadOnly);
		stream >> uid >> item_id >> options >> operate_time;
		res = query.exec(QString("INSERT INTO magic_apply (uid, item_id, options, status, operate_time) VALUES ('%1', '%2', '%3', '%4', '%5')").arg(uid, item_id, options, QString("0"), operate_time));
		qDebug() << query.lastError().text();
	}else
	{
		res = query.exec(QString("DELETE FROM magic_apply WHERE apply_id='%1'").arg(apply_id));
	}
	query.clear();
	DB.close();
	
	emit submitOrCancelApplyFinished(res);
}

void ApprovalWork::authApplyToken(const QString& token)
{
	DB_SECOND.open();
	QSqlQuery query(DB_SECOND);
	query.exec(QString("SELECT * FROM magic_apply WHERE token = '%1'").arg(token));
	bool res = query.next();
	if (res)
	{
		authApplyTokenResultList.clear();
		QString apply_id = query.value("apply_id").toString();
		QString uid = query.value("uid").toString();
		QString item_id = query.value("item_id").toString();
		QString status = query.value("status").toString();

		if(status == "0")
			status = "待审核";
		else if (status == "1" || status == "3")
			status = "已通过";
		else if (status == "2")
			status = "已终止（未通过）";
		QString operate_time = query.value("operate_time").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
		QString item = QString("[%1] %2").arg(item_id, applyItemTitle[item_id]);
		authApplyTokenResultList << apply_id << uid << item << status << operate_time << token;
		emit authApplyTokenFinished(res);
	}
	else
		emit authApplyTokenFinished(res);
	query.clear();
	DB_SECOND.close();
}

QList<QByteArray> ApprovalWork::getCurrentApplyProcess(const QString& id)
{
	return applyFormsProcess[id];
}

QString ApprovalWork::getCurrentFormOptionsText(const QString& id)
{
	return currentFormOptionsText[id];
}

QList<QString> ApprovalWork::getAuthApplyTokenResultList()
{
	return authApplyTokenResultList;
}

void ApprovalWork::setSmtpConfig(const QList<QString> config)
{
	smtp_config = config;
}

ApprovalWork::~ApprovalWork()
{}
