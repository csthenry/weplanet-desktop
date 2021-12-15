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
        return false;
    }
    else{
        QSqlQuery query;
        //创建magic_users表
        //初始化root用户数据，登录名100000，密码123456
        QString creatTableStr = "CREATE TABLE IF NOT EXISTS magic_users   \
                (                                           \
                  uid           int(10)      NOT NULL AUTO_INCREMENT,     \
                  password      varchar(64)  NOT NULL ,         \
                  name          varchar(32)  NOT NULL ,     \
                  gender        char(1)      NULL ,         \
                  telephone     varchar(64)  NULL ,         \
                  mail          varchar(128) NULL ,         \
                  user_group         char(1) NOT NULL ,         \
                  user_position      char(1)      NULL ,         \
                  PRIMARY KEY (uid)             \
                  )ENGINE=InnoDB;               \
                  INSERT INTO magic_users       \
                  ( uid, password, name, user_group )       \
                  VALUES                        \
                  (                            \
                100000, '" + service::pwdEncrypt("123456") + "', 'Henry', '1'    \
                  )";
        query.prepare(creatTableStr);
        query.exec();

        //用户组权限分配表 待设计。。。

        //团队架构表
        creatTableStr =
                  "CREATE TABLE IF NOT EXISTS magic_department"
                  "(dpt_id      int(10)      NOT NULL    AUTO_INCREMENT,"
                  "dpt_name     varchar(32)  NOT NUll,"
                  "PRIMARY KEY (dpt_id))ENGINE=InnoDB";
        query.prepare(creatTableStr);
        query.exec();

        //考勤表
        creatTableStr =
                  "CREATE TABLE IF NOT EXISTS magic_attendance"
                  "(num      int(10)      NOT NULL    AUTO_INCREMENT,"
                  "uid       int(10)      NOT NUll,"
                  "begin_date   datetime,"
                  "end_date     datetime,"
                  "today        date,"
                  "isSupply     char(1),"
                  "supply_adminUid  int(10),"
                  "PRIMARY KEY (num, uid)"
                  ")ENGINE=InnoDB;";
        query.prepare(creatTableStr);
        query.exec();

        //申请表
        creatTableStr =
                  "CREATE TABLE IF NOT EXISTS magic_apply"
                  "(apply_id      int(10)      NOT NULL    AUTO_INCREMENT,"
                  "uid            int(10)      NOT NUll,"
                  "alist_id       int(10)      NOT NUll,"
                  "op1_text       text,"
                  "op2_text       text,"
                  "op3_text       text,"
                  "process_uidList varchar(64),"
                  "status         char(1)     NOT NUll,"
                  "check_uidList  varchar(64),"
                  "reject_uid     int(10),"
                  "PRIMARY KEY (apply_id, uid)"
                  ")ENGINE=InnoDB;";
        query.prepare(creatTableStr);
        query.exec();

        //审批流程项目表
        creatTableStr =
                  "CREATE TABLE IF NOT EXISTS magic_applyList"
                  "(alist_id      int(10)      NOT NULL    AUTO_INCREMENT,"
                  "title          varchar(32)  NOT NUll,"
                  "op1_title      varchar(32),"
                  "op2_title      varchar(32),"
                  "op3_title      varchar(32),"
                  "option_num         char(1),"
                  "PRIMARY KEY (alist_id)"
                  ")ENGINE=InnoDB;";
        query.prepare(creatTableStr);
        query.exec();

        //用户认证信息表
        creatTableStr =
                  "CREATE TABLE IF NOT EXISTS magic_verify"
                  "(uid      int(10)      NOT NULL    AUTO_INCREMENT,"
                  "verify_name          varchar(32)  NOT NUll,"
                  "info      varchar(32),"
                  "icon         char(1),"
                  "PRIMARY KEY (uid)"
                  ")ENGINE=InnoDB;";
        query.prepare(creatTableStr);
        query.exec();
        return true;
    }
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

