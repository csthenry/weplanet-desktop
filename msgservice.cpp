#include "msgservice.h"

MsgService::MsgService(QObject *parent, int path)
	: QObject(parent)
{
	db_service.addDatabase(DB, "MsgService_DB_" + QString::number(path)); 
	db_service.addDatabase(DB_PUSHER, "MsgService_DB_PUSHER_" + QString::number(path));
}

void MsgService::loadMsgMemList(QString uid)
{
	DB.open();
	QSqlQuery query(DB);
	query.exec("SELECT * FROM magic_system WHERE sys_name='openChat';");
	if (query.next())
	{
		QSqlRecord record = query.record();
		isOpen = record.value("field_1").toBool();
		if(isOpen)
			getMsgMem(uid);
	}
	else
		isOpen = false;
	emit loadMsgMemListFinished();
}

void MsgService::sendMessage(QByteArray array)
{
	QString fromUid, toUid, msgText, curDatetime;
	DB.open();
	QSqlQuery query(DB);
	QDataStream stream(&array, QIODevice::ReadOnly);
	stream >> fromUid >> toUid >> msgText >> curDatetime;
	
	bool res = query.exec(QString("INSERT INTO magic_message (from_uid, to_uid, text, send_time) VALUES ('%1','%2','%3','%4')").arg(fromUid, toUid, msgText, curDatetime));

	query.clear();
	DB.close();
	emit sendMessageFinished(res);
}

void MsgService::pushMessage(QString me, QString member, int limit)
{
	pushingUid = member;
	if (member == "-1")
		return;
	if (msgStackCnt.contains(member))	//初始化对应用户消息栈数据量
		msgStackCnt.value(member, 0);
	int cnt = 1;
	DB_PUSHER.open();
	QSqlQuery query(DB_PUSHER);
	QString from_uid, from_name, to_uid, to_name, msgText, send_time;

	msgStack.clear();
	query.exec(QString("SELECT * FROM magic_message WHERE from_uid='%1' AND to_uid='%2' UNION ALL SELECT * FROM magic_message WHERE from_uid='%2' AND to_uid='%1' ORDER BY id DESC").arg(me, member));	   // LIMIT 0,30
	msgStackCnt[member] = query.size();	//消息栈数据量
	while (query.next() && cnt <= limit) {
		QByteArray array;
		QDataStream stream(&array, QIODevice::WriteOnly);
		QSqlRecord record = query.record();
		array.clear();
		from_uid = record.value("from_uid").toString();
		to_uid = record.value("to_uid").toString();
		from_name = getName(from_uid, 2);
		to_name = getName(to_uid, 2);
		msgText = record.value("text").toString();
		send_time = record.value("send_time").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
		stream << from_uid << from_name << to_uid << to_name << msgText << send_time;
		msgStack.push(array);
		cnt++;
	}
	query.clear();
	//在线状态检测
	query.exec(QString("INSERT INTO magic_online (uid, latest) VALUES ('%1', '%2') ON DUPLICATE KEY UPDATE latest = '%2';").arg(me, SecsSinceEpoch));	//更新自己的状态
	query.exec(QString("SELECT * FROM magic_online WHERE uid = '%1';").arg(member));
	if (query.next())
	{
		QSqlRecord record = query.record();
		int latest = record.value("latest").toInt();
		if (SecsSinceEpoch.toInt() - latest > 120)	//120s的检测间隔，超过则代表下线
			isOnline.insert(member, false);
		else
			isOnline.insert(member, true);
	}
	else
		isOnline.insert(member, false);
	query.exec();
	DB_PUSHER.close();

	previousPushUid = member;
	emit pusher(msgStack);
}

void MsgService::searchMember(const QString& uid)
{
	DB.open();
	QSqlQuery query(DB);
	QSqlRecord record;
	QString name, group, department;
	QPixmap avatar;
	QByteArray array;
	QDataStream stream(&array, QIODevice::WriteOnly);

	query.exec("SELECT * FROM magic_users WHERE uid='" + uid + "'");
	bool res = query.next();
	record = query.record();
	name = record.value("name").toString();
	group = db_service.getGroup(DB, uid);
	department = db_service.getDepartment(DB, uid);
	if (db_service.getAvatar(record.value("user_avatar").toString()).isNull())
		avatar = QPixmap(":/images/color_icon/user.svg");
	else
		avatar = db_service.getAvatar(record.value("user_avatar").toString());

	stream << name << group << department << avatar << res;
	query.clear();
	DB.close();
	emit searchMemRes(array);
}

void MsgService::loadApplyInfo(const QString& me, const QString& member)
{
	DB.open();
	QSqlQuery query(DB);
	query.exec(QString("SELECT extra AS info FROM magic_relation WHERE user_id='%2' AND friend_id='%1' AND status='%3'").arg(me, member, "0"));
	if(query.next())
		emit loadApplyInfoFinished(query.record().value("info").toString());
	else
		emit loadApplyInfoFinished("错误信息：" + query.lastError().text());
	query.clear();
	DB.close();
}

void MsgService::sendApply(const QString& me, const QString& member, const QString& info)
{
	DB.open();
	QSqlQuery query(DB);
	QString res = "未知错误，请检查网络或联系管理员。\n错误信息：";
	query.exec(QString("SELECT * FROM magic_relation WHERE (user_id='%1' AND friend_id='%2') OR (user_id='%2' AND friend_id='%1') ").arg(me, member));
	if (query.next())
	{
		res = "Exist";
		emit sendApplyFinished(res);
		return;
	}
	query.clear();
	if (query.exec(QString("INSERT INTO magic_relation (user_id, friend_id, extra, status) VALUES ('%1', '%2', '%3', '%4')").arg(me, member, info, "0")))
		res = "OK";
	else
		res += query.lastError().text();
	query.clear();
	DB.close();
	emit sendApplyFinished(res);
}

void MsgService::operateApply(const QString& me, const QString& member, int flag)
{
	DB.open();
	QSqlQuery query(DB);
	bool queryRes = false;
	QString res = "未知错误，请检查网络或联系管理员。\n错误信息：";
	if(flag == 1)
		queryRes = query.exec(QString("UPDATE magic_relation SET status='1' WHERE user_id='%2' AND friend_id='%1'").arg(me, member));
	else
		queryRes = query.exec(QString("DELETE FROM magic_relation WHERE user_id='%2' AND friend_id='%1'").arg(me, member));
	if (queryRes)
		res = "OK_" + QString::number(flag);
	else
		res += query.lastError().text();

	query.clear();
	DB.close();
	emit operateApplyFinished(res);
}

void MsgService::delFriend(const QString& me, const QString& member)
{
	DB.open();
	bool queryRes = false;
	QSqlQuery query(DB);
	QString res = "未知错误，请检查网络或联系管理员。\n错误信息：";
	queryRes = query.exec(QString("DELETE FROM magic_relation WHERE (user_id='%1' AND friend_id='%2' AND status='1') OR (user_id='%2' AND friend_id='%1' AND status='1')").arg(me, member));
	if (queryRes)
		res = "OK";
	else
		res += query.lastError().text();
	query.clear();
	DB.close();

	emit delFriendFinished(res);
}

bool MsgService::getIsOpen()
{
	return isOpen;
}

bool MsgService::getIsOnline(const QString& member)
{
	if (isOnline.contains(member))
		return isOnline[member];
	else
		return false;
}

int MsgService::getMsgStackCnt(const QString& uid)
{
	return msgStackCnt[uid];
}

QList<QString> MsgService::getMsgMem(QString uid)
{
	DB.open();
	QSqlQuery query(DB);
	msgMem.clear();
	msgMemName.clear();
	avatar.clear();
	msgApplyMem.clear();
	applyAvatar.clear();
	msgApplyMemName.clear();

	query.exec("SELECT friend_id AS friends FROM magic_relation WHERE user_id='" + uid + "' AND status=1 UNION ALL SELECT user_id AS friends FROM magic_relation WHERE friend_id='" + uid + "' AND status=1");
	while (query.next()) {
		QSqlRecord record = query.record();
		msgMem.append(record.value("friends").toString());
		msgMemName.append(getName(record.value("friends").toString(), 1));
		getAvatar(record.value("friends").toString(), 1);
	}
	query.clear();
	query.exec("SELECT user_id AS applies FROM magic_relation WHERE friend_id='" + uid + "' AND status=0");
	while (query.next()) {
		QSqlRecord record = query.record();
		msgApplyMem.append(record.value("applies").toString());
		msgApplyMemName.append(getName(record.value("applies").toString(), 1));
		getAvatar(record.value("applies").toString(), 2);
	}
	DB.close();
	return msgMem;
}

void MsgService::getAvatar(const QString& member, int path)
{
	//调用函数前请打开数据库
	QSqlQuery query(DB);
	query.exec("SELECT user_avatar FROM magic_users WHERE uid='" + member + "'");
	query.next();
	if (path == 1)
	{
		if (db_service.getAvatar(query.record().value(0).toString()).isNull())
			avatar.append(QPixmap(":/images/color_icon/user.svg"));
		else
			avatar.append(db_service.getAvatar(query.record().value(0).toString()));
	}
	else
	{
		if (db_service.getAvatar(query.record().value(0).toString()).isNull())
			applyAvatar.append(QPixmap(":/images/color_icon/user.svg"));
		else
			applyAvatar.append(db_service.getAvatar(query.record().value(0).toString()));
	}
}

QString MsgService::getName(const QString& uid, int path)
{
	//调用函数前请打开数据库
	QSqlQuery query;
	if (path == 1)
	{
		QSqlQuery query_1(DB);
		query = query_1;
	}
	else
	{
		QSqlQuery query_2(DB_PUSHER);
		query = query_2;
	}
	QSqlRecord record;
	query.exec("SELECT name FROM magic_users WHERE uid='" + uid + "'");
	query.next();
	record = query.record();
	query.clear();
	return record.value("name").toString();
}

QList<QString> MsgService::getMsgMemList()
{
	return msgMem;
}

QList<QString> MsgService::getMsgMemNameList()
{
	return msgMemName;
}

QList<QPixmap> MsgService::getAvatarList()
{
	return avatar;
}

QList<QString> MsgService::getMsgApplyMemList()
{
	return msgApplyMem;
}

QList<QString> MsgService::getMsgApplyMemNameList()
{
	return msgApplyMemName;
}

QList<QPixmap> MsgService::getApplyAvatarList()
{
	return applyAvatar;
}

QString MsgService::getPushingUid()
{
	return pushingUid;
}

QString MsgService::getPreviousPushUid()
{
	return previousPushUid;
}

MsgService::~MsgService()
{
}
