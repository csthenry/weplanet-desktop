#include "msgservice.h"

MsgService::MsgService(QObject *parent)
	: QObject(parent)
{
	db_service.addDatabase(DB, "MsgService_DB");
}

void MsgService::loadMsgMemList(QString uid)
{
	msgMemName.clear();
	getMsgMem(uid);
	for (auto cur : msgMem)
		msgMemName.append(getName(cur));

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

void MsgService::pushMessage(QString uid)
{
	if (uid == "-1")
		return;
	DB.open();
	QSqlQuery query(DB);
	QString from_uid, from_name, to_uid, to_name, msgText, send_time;

	msgStack.clear();
	query.exec("SELECT * FROM magic_message WHERE from_uid='" + uid + "' UNION ALL SELECT * FROM magic_message WHERE to_uid='" + uid + "' ORDER BY send_time DESC LIMIT 0,30");
	while (query.next()) {
		QByteArray array;
		QDataStream stream(&array, QIODevice::WriteOnly);
		QSqlRecord record = query.record();
		array.clear();
		from_uid = record.value("from_uid").toString();
		to_uid = record.value("to_uid").toString();
		from_name = getName(from_uid);
		to_name = getName(to_uid);
		msgText = record.value("text").toString();
		send_time = record.value("send_time").toDateTime().toString("yyyy-MM-dd hh:mm:ss");
		stream << from_uid << from_name << to_uid << to_name << msgText << send_time;
		msgStack.push(array);
	}
	query.clear();
	DB.close();

	emit pusher(msgStack);
}

QList<QString> MsgService::getMsgMem(QString uid)
{
	DB.open();
	QSqlQuery query(DB);
	msgMem.clear();
	query.exec("SELECT friend_id AS friends FROM magic_relation WHERE user_id='" + uid + "' UNION ALL SELECT user_id AS friends FROM magic_relation WHERE friend_id='" + uid + "'");
	while (query.next()) {
		QSqlRecord record = query.record();
		msgMem.append(record.value("friends").toString());
	}
	getAvatar(msgMem);
	query.clear();
	DB.close();
	return msgMem;
}

void MsgService::getAvatar(QList<QString>& members)
{
	//调用函数前请打开数据库
	QSqlQuery query(DB);
	avatar.clear();
	for (auto curMem : members)
	{
		query.exec("SELECT user_avatar FROM magic_users WHERE uid='" + curMem + "'");
		query.next();
		if (db_service.getAvatar(query.record().value(0).toString()).isNull())
			avatar.append(QPixmap(":/images/color_icon/user.svg"));
		else
			avatar.append(db_service.getAvatar(query.record().value(0).toString()));
	}
}

QString MsgService::getName(QString uid)
{
	DB.open();
	QSqlQuery query(DB);
	QSqlRecord record;
	query.exec("SELECT name FROM magic_users WHERE uid='" + uid + "'");
	query.next();
	record = query.record();
	query.clear();
	DB.close();
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

MsgService::~MsgService()
{
}
