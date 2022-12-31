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
	service db_service;
	QSqlDatabase DB;
	QList<QString> msgMem;
	QList<QString> msgMemName;
	QList<QPixmap> avatar;
	QList<QString> getMsgMem(QString uid);
	QStack<QByteArray> msgStack;
	void getAvatar(QList<QString>& members);
	QString getName(QString uid);

public:
	QList<QString> getMsgMemList();
	QList<QString> getMsgMemNameList();
	QList<QPixmap> getAvatarList();
	void loadMsgMemList(QString uid);
	void sendMessage(QByteArray array);
	void pushMessage(QString uid);

public:
	MsgService(QObject *parent = nullptr);
	~MsgService();

private slots:

signals:
	void loadMsgMemListFinished();
	void sendMessageFinished(bool);
	void pusher(QStack<QByteArray>);
};

#endif //MSGSERVICE_H
