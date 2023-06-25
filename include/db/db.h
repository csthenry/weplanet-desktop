#pragma once
#pragma execution_character_set("utf-8")

#ifndef DB_H
#define DB_H

#include <QtSql>

#define MYSQL_TIME_OUT 180000     //定义数据库心跳时间ms

class MySQL
{
public:
	QString connectionName;

	//void setHostName(const QString& hostName);
	//void setDataBasePort(int dataBasePort);
	//void setDataBaseName(const QString& dataBaseName);
	//void setDataBaseUserName(const QString& dataBaseUserName);
	//void setDataBasePassword(const QString& dataBasePassword);
	//void setDatabaseType(const QString& dataBaseType);
	//QSqlDatabase getDb() const;

	bool connectDb(QString& errorMsg, const QString& flagEx = QString());
	void disconnectDb();

	bool insert(const QString& table, const QStringList& fields, const QStringList& values);
	bool update(const QString& table, const QStringList& fields, const QStringList& values, const QString& condition);
	bool remove(const QString& table, const QString& condition);
	QSqlQuery select(const QString& table, const QStringList& fields, const QString& condition);
	QSqlQuery select(const QString& table, const QStringList& fields, const QString& condition, const QString& order);
	QSqlQuery select(const QString& table, const QStringList& fields, const QString& condition, const QString& order, const QString& limit);
	MySQL();
private:
	QString dataBaseType;
	QString hostName;
	int dataBasePort;
	QString dataBaseName;
	QString dataBaseUserName;
	QString dataBasePassword;
	//QSqlDatabase db;
};

#endif // DB_H