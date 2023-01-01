#pragma once
#pragma execution_character_set("utf-8")

#ifndef MSGSERVICE_H
#define MSGSERVICE_H

#include <QObject>
#include <QStack>
#include "service.h"

class MsgService  : public QObject
{
	Q_OBJECT

private:
	QHash<QString, int> msgStackCnt;	//当前会话消息栈数据量
	service db_service;
	QSqlDatabase DB, DB_PUSHER;
	QList<QString> msgMem, msgApplyMem;
	QList<QString> msgMemName, msgApplyMemName;
	QList<QPixmap> avatar, applyAvatar;
	QList<QString> getMsgMem(QString uid);
	QStack<QByteArray> msgStack;
	void getAvatar(const QString& member, int path);
	QString getName(const QString& uid, int path);

public:
	QList<QString> getMsgMemList();
	QList<QString> getMsgMemNameList();
	QList<QPixmap> getAvatarList();
	QList<QString> getMsgApplyMemList();
	QList<QString> getMsgApplyMemNameList();
	QList<QPixmap> getApplyAvatarList();
	void loadMsgMemList(QString uid);
	void sendMessage(QByteArray array);
	void pushMessage(QString uid, int limit);
	void searchMember(const QString& uid);
	void loadApplyInfo(const QString& me, const QString& member);
	void sendApply(const QString& me, const QString& member, const QString& info);
	int getMsgStackCnt(const QString& uid);
	void operateApply(const QString& me, const QString& member, int flag);
	void delFriend(const QString& me, const QString& member);

public:
	MsgService(QObject *parent = nullptr, int path = 1);
	~MsgService();

private slots:

signals:
	void loadMsgMemListFinished();
	void sendMessageFinished(bool);
	void searchMemRes(QByteArray);
	void pusher(QStack<QByteArray>);
	void sendApplyFinished(QString res);
	void loadApplyInfoFinished(QString info);
	void operateApplyFinished(QString info);
	void delFriendFinished(QString info);
};

#endif //MSGSERVICE_H
