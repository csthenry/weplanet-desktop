#include "service.h"
#include <QDebug>

service::service()
{
    /*****************请在此处完善数据库信息*****************/

    dataBaseType = "QMYSQL";    //available drivers: QSQLITE QMYSQL QMYSQL3 QODBC QODBC3 QPSQL QPSQL7
    hostName = "localhost";
    dataBasePort = 3306;
    dataBaseName = "magic";
    dataBaseUserName = "root";
    dataBasePassword = "123456";

    /*****************请在此处完善数据库信息*****************/
}

QString service::pwdEncrypt(const QString &str) //字符串MD5算法加密
{
    QByteArray btArray;
    btArray.append(str);//加入原始字符串
    QCryptographicHash hash(QCryptographicHash::Md5);  //Md5加密算法
    hash.addData(btArray);  //添加数据到加密哈希值
    QByteArray resultArray =hash.result();  //返回最终的哈希值
    QString md5 = resultArray.toHex();  //转换为16进制字符串
    return  md5;
}

void service::connectDatabase(QSqlDatabase& db)
{
    //如果存在连接名为qt_sql_default_connection（默认连接名）的连接，则不需要addDatabase
    //否则会报出QSqlDatabasePrivate::addDatabase: duplicate connection name 'qt_sql_default_connect...
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase(dataBaseType);
    db.setHostName(hostName);
    db.setPort(dataBasePort);
    db.setDatabaseName(dataBaseName);
    db.setUserName(dataBaseUserName);
    db.setPassword(dataBasePassword);
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
        QString creatTableStr = "CREATE TABLE IF NOT EXISTS magic_users "
                 "(                                                     "
                 "uid           int(10)      NOT NULL AUTO_INCREMENT,   "
                 "password      varchar(64)  NOT NULL ,"
                 "name          varchar(32)  NULL ,"
                 "gender        tinytext     NULL ,"
                 "telephone     varchar(64)  NULL ,"
                 "mail          varchar(128) NULL ,"
                 "user_group    int(10) NOT  NULL ,"
                 "user_dpt      int(10) NOT  NULL ,"
                 "user_avatar   varchar(64)  NULL ,"
                 "PRIMARY KEY (uid)                "
                 ")ENGINE=InnoDB;                  "
                 "INSERT INTO magic_users          "
                 "(uid, password, name, user_group)"
                 "VALUES                           "
                 "(                                "
                 "100000, '" + service::pwdEncrypt("123456") + "', 'Henry', '1')";
        query.prepare(creatTableStr);
        query.exec();

        //用户组权限分配表，默认有管理员和普通用户
        creatTableStr =
                  "CREATE TABLE IF NOT EXISTS magic_group"
                  "(group_id        int(10)     NOT NULL    AUTO_INCREMENT,"
                  "group_name      varchar(32)  NOT NULL,"
                  "users_manage     tinyint(1)  NOT NUll,"
                  "attend_manage    tinyint(1)  NOT NUll,"
                  "apply_manage     tinyint(1)  NOT NUll,"
                  "applyItem_manage tinyint(1)  NOT NUll,"
                  "group_manage     tinyint(1)  NOT NUll,"
                  "activity_manage  tinyint(1)  NOT NUll,"
                  "send_message     tinyint(1)  NOT NUll,"
                  "PRIMARY KEY (group_id)) ENGINE=InnoDB;"
                  "INSERT INTO magic_group"
                  "(group_id, group_name, users_manage, attend_manage, apply_manage, applyItem_manage, group_manage, activity_manage, send_message)"
                  "VALUES(1, '超级管理员', 1, 1, 1, 1, 1, 1, 1);"
                  "INSERT INTO magic_group"
                  "(group_id, group_name, users_manage, attend_manage, apply_manage, applyItem_manage, group_manage, activity_manage, send_message)"
                  "VALUES(2, '普通用户', 0, 0, 0, 0, 0, 0, 1);";
        query.prepare(creatTableStr);
        query.exec();

        //组织架构表
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
                  "(apply_id       int(10)      NOT NULL    AUTO_INCREMENT,"
                  "uid             int(10)      NOT NUll,"
                  "alist_id        int(10)      NOT NUll,"
                  "op1_text        text,"
                  "op2_text        text,"
                  "op3_text        text,"
                  "process_uidList varchar(64),"
                  "status          tinyint(1)   NOT NUll,"
                  "check_uidList   varchar(64),"
                  "reject_uid      int(10),"
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
                  "option_num     tinyint(1), "
                  "PRIMARY KEY (alist_id)"
                  ")ENGINE=InnoDB;";
        query.prepare(creatTableStr);
        query.exec();

        //用户认证信息表
        creatTableStr =
                  "CREATE TABLE IF NOT EXISTS magic_verify"
                  "(uid            int(10)      NOT NULL    AUTO_INCREMENT,"
                  "verify_name     varchar(32)  NOT NUll,"
                  "info            varchar(32),"
                  "icon            char(1),"
                  "PRIMARY KEY (uid)"
                  ")ENGINE=InnoDB;";
        query.prepare(creatTableStr);
        query.exec();
        return true;
    }
}

bool service::authAccount(QSqlDatabase& db, QString& uid, const long long account, const QString& pwd)
{
    if(!db.open())
        return false;
    QSqlQuery query;
    //验证UID
    query.exec("SELECT password FROM magic_users WHERE uid = " + QString::number(account));
    if(query.next() && pwd == query.value("password").toString())
    {
        uid = QString::number(account);
        qDebug() << uid << "登录方式：账号登录";
        return true;
    }
    else
    {
        //验证手机号，可能有重复的手机号，所以用while
        query.exec("SELECT uid, password FROM magic_users WHERE telephone = " + QString::number(account));
        while(query.next())
            if(pwd == query.value("password").toString())
            {
                uid = query.value("uid").toString();
                qDebug() << uid << "登录方式：手机号登录";
                return true;
            }
        return false;
    }
}

QPixmap service::getAvatar(const QString& url)
{
    QUrl picUrl(url);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply = manager.get(QNetworkRequest(url));
    //请求结束并下载完成后，退出子事件循环
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();
    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);
    return pixmap;
}

QPixmap service::setAvatarStyle(QPixmap avatar)
{
    QPixmap pixmapa(avatar), pixmap(120,120);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, 120, 120);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, 120, 120, pixmapa);
    return pixmap;
}

QString service::getGroup(const QString& uid)
{
    QSqlQuery query;
    query.exec("SELECT user_group FROM magic_users WHERE uid = " + uid);
    if(!query.next())
        return "--";
    query.exec("SELECT group_name FROM magic_group WHERE group_id = " + query.value("user_group").toString());
    if(!query.next())
        return "--";
    return query.value("group_name").toString();
}

QString service::getDepartment(const QString& uid)
{
    QSqlQuery query;
    query.exec("SELECT user_dpt FROM magic_users WHERE uid = " + uid);
    if(!query.next())
        return "--";
    query.exec("SELECT dpt_name FROM magic_department WHERE dpt_id = " + query.value("user_dpt").toString());
    if(!query.next())
        return "--";
    return query.value("dpt_name").toString();
}

