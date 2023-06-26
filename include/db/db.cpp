#include "db.h"

MySQL::MySQL()
{
	/*****************请在此处完善数据库信息*****************/

	dataBaseType = "QMYSQL";    //available drivers: QSQLITE QMYSQL QMYSQL3 QODBC QODBC3 QPSQL QPSQL7
	hostName = "api.bytecho.net";
	dataBasePort = 3306;
	dataBaseName = "magic";
	dataBaseUserName = "magic";
	dataBasePassword = "*************";

	/*****************请在此处完善数据库信息*****************/
}

bool MySQL::connectDb(QString& errorMsg, const QString& flagEx)
{
	connectionName = QString::number((quint64)QThread::currentThreadId());
	if(!flagEx.isEmpty())
		connectionName += flagEx;
	QSqlDatabase db = QSqlDatabase::addDatabase(dataBaseType, connectionName);
	db.setHostName(hostName);         //主机名
	db.setPort(dataBasePort);         //端口
	db.setDatabaseName(dataBaseName); //数据库名
	db.setUserName(dataBaseUserName); //用户名
	db.setPassword(dataBasePassword); //密码
	if (!db.open())
	{
		errorMsg = db.lastError().text();
		qDebug() << "open database failed:" << errorMsg;
		return false;
	}

	return true;
}

void MySQL::disconnectDb()
{
	// https://doc.qt.io/qt-6/qsqldatabase.html#removeDatabase
	{
		QSqlDatabase dbConnected = QSqlDatabase::database(connectionName);
		if (dbConnected.isValid())
			dbConnected.close();
	}
	QSqlDatabase::removeDatabase(connectionName);
	//qDebug() << "[database removed] db: " << connectionName;
}

bool MySQL::insert(const QString& table, const QStringList& fields, const QStringList& values)
{
	QSqlDatabase db = QSqlDatabase::database(connectionName);
	if(db.isOpen() == false || !db.isValid())
		return false;
	if(fields.size() != values.size())
		return false;
	QSqlQuery query(db);
	QString sql = "insert into " + table + "(" + fields.join(",") + ") values(" + values.join(",") + ")";
	return query.exec(sql);
}

bool MySQL::update(const QString& table, const QStringList& fields, const QStringList& values, const QString& condition)
{
	QSqlDatabase db = QSqlDatabase::database(connectionName);
	if(db.isOpen() == false || !db.isValid())
		return false;
	if(fields.size() != values.size())
		return false;
	QSqlQuery query(db);
	QString sql = "update " + table + " set ";
	for (int i = 0; i < fields.size(); i++)
	{
		sql += fields[i] + "=" + values[i];
		if(i != fields.size() - 1)
			sql += ",";
	}
	sql += " where " + condition;
	return query.exec(sql);
}

bool MySQL::remove(const QString& table, const QString& condition)
{
	QSqlDatabase db = QSqlDatabase::database(connectionName);
	if(db.isOpen() == false || !db.isValid())
		return false;
	QSqlQuery query(db);
	QString sql = "delete from " + table + " where " + condition;
	return query.exec(sql);
}

QSqlQuery MySQL::select(const QString& table, const QStringList& fields, const QString& condition)
{
	QSqlDatabase db = QSqlDatabase::database(connectionName);
	if(db.isOpen() == false || !db.isValid())
		return QSqlQuery();
	QSqlQuery query(db);
	QString sql = "select ";
	if(fields.size() == 0)
		sql += "*";
	else
		sql += fields.join(",");
	sql += " from " + table;
	if(condition != "")
		sql += " where " + condition;
	query.exec(sql);
	return query;
}

QSqlQuery MySQL::select(const QString& table, const QStringList& fields, const QString& condition, const QString& order)
{
	QSqlDatabase db = QSqlDatabase::database(connectionName);
	if(db.isOpen() == false || !db.isValid())
		return QSqlQuery();
	QSqlQuery query(db);
	QString sql = "select ";
	if(fields.size() == 0)
		sql += "*";
	else
		sql += fields.join(",");
	sql += " from " + table;
	if(condition != "")
		sql += " where " + condition;
	if(order != "")
		sql += " order by " + order;
	query.exec(sql);
	return query;
}

QSqlQuery MySQL::select(const QString& table, const QStringList& fields, const QString& condition, const QString& order, const QString& limit)
{
	QSqlDatabase db = QSqlDatabase::database(connectionName);
	if (db.isOpen() == false || !db.isValid())
		return QSqlQuery();
	QSqlQuery query(db);
	QString sql = "select ";
	if (fields.size() == 0)
		sql += "*";
	else
		sql += fields.join(",");
	sql += " from " + table;
	if (condition != "")
		sql += " where " + condition;
	if (order != "")
		sql += " order by " + order;
	if (limit != "")
		sql += " limit " + limit;
	query.exec(sql);
	return query;
}