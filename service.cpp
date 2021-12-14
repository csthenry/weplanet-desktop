#include "service.h"
#include <QDebug>

service::service()
{

}

QString service::pwdEncrypt(const QString &str)//字符串MD5算法加密
{
    QByteArray btArray;
    btArray.append(str);//加入原始字符串
    QCryptographicHash hash(QCryptographicHash::Md5);  //Md5加密算法
    hash.addData(btArray);  //添加数据到加密哈希值
    QByteArray resultArray =hash.result();  //返回最终的哈希值
    QString md5 = resultArray.toHex();//转换为16进制字符串
    return  md5;
}

void service::connectDatabase(QSqlDatabase& db)
{
    //如果存在连接名为qt_sql_default_connection（默认连接名）的连接，则不需要addDatabase
    //否则会报出QSqlDatabasePrivate::addDatabase: duplicate connection name 'qt_sql_default_connect...
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("magic");
    db.setUserName("root");
    db.setPassword("123456");
}

bool service::initDatabaseTables(QSqlDatabase& db)
{

    if(!db.open()){
        qDebug() << "error info :" << db.lastError();
    }
    else{
        QSqlQuery query;
        //创建magic_users表
        QString creatTableStr = "CREATE TABLE magic_users   \
                (                                           \
                  uid           int(10)      NOT NULL AUTO_INCREMENT,     \
                  password      varchar(64)  NOT NULL ,         \
                  name          varchar(32)  NOT NULL ,     \
                  gender        char(1)      NULL ,         \
                  telephone     varchar(64)  NULL ,         \
                  mail          varchar(128) NULL ,         \
                  user_group         char(1)      NULL ,         \
                  user_position      char(1)      NULL ,         \
                  PRIMARY KEY (uid)           \
                )ENGINE=InnoDB;";

        query.prepare(creatTableStr);

        //初始化root用户数据，登录名100000，密码123456
        creatTableStr = "INSERT INTO magic_users    \
                        ( uid, password, name )     \
                        VALUES                      \
                        (                           \
                        100000, '" + service::pwdEncrypt("123456") + "', 'Henry'    \
                        );";
        query.prepare(creatTableStr);

        if(!query.exec()){
            qDebug() << query.lastError().text();
            return false;
        }
        else{
            return true;
        }
    }
    return false;
}

bool service::authAccount(QSqlDatabase& db, int uid, QString pwd)
{
    if(!db.open())
        return false;
    QSqlQuery query;
    query.exec("SELECT password FROM magic_users; WHERE uid = " + QString::number(uid));
    if(query.next() && pwd == query.value(0))
        return true;
    else
    {
        qDebug() << query.lastError().text();
        return false;
    }
}

