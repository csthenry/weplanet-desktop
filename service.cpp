/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "service.h"
#include <QDebug>

service::service()
{
    /*****************请在此处完善数据库信息*****************/

    dataBaseType = "QMYSQL";    //available drivers: QSQLITE QMYSQL QMYSQL3 QODBC QODBC3 QPSQL QPSQL7
    hostName = "106.54.176.177";
    dataBasePort = 3306;
    dataBaseName = "magic";
    dataBaseUserName = "magic";
    dataBasePassword = "fPHHs6D7rsP8cztP";

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
    return md5;
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

void service::addDatabase(QSqlDatabase& db, const QString &flag)
{
    if (QSqlDatabase::contains(flag))
        db = QSqlDatabase::database(flag);
    else
        db = QSqlDatabase::addDatabase(dataBaseType, flag);
    db.setHostName(hostName);
    db.setPort(dataBasePort);
    db.setDatabaseName(dataBaseName);
    db.setUserName(dataBaseUserName);
    db.setPassword(dataBasePassword);
}

bool service::initDatabaseTables(QSqlDatabase db)
{
    qDebug() << "initDatabaseTables()...";
    bool res = true;
    QSqlQuery query(db);
    //创建magic_users表
    //初始化root用户数据，登录名100000，密码123456
    QString creatTableStr = "CREATE TABLE IF NOT EXISTS magic_users "
        "(                                                     "
        "uid           int(10)      NOT NULL AUTO_INCREMENT,   "
        "password      varchar(64)  NOT NULL ,"
        "name          varchar(32)  NOT NULL ,"
        "gender        tinytext     NULL ,"
        "telephone     varchar(64)  NULL ,"
        "mail          varchar(128) NULL ,"
        "user_group    int(10) NOT  NULL ,"
        "user_dpt      int(10) NOT  NULL ,"
        "user_avatar   varchar(256) NULL ,"
        "score         float(5,2)   NUll ,"
        "user_status   tinyint(1)   NOT NULL ,"
        "PRIMARY KEY (uid)                "
        ")ENGINE=InnoDB;                  "
        "INSERT INTO magic_users          "
        "(uid, password, name, user_group, user_dpt, score, user_status)"
        "VALUES(1, 'kH9bV0rP5dF8oW7g', '系统', 2, 1, 0, 0);"
        "INSERT INTO magic_users          "
        "(uid, password, name, user_group, user_dpt, score, user_status)"
        "VALUES                           "
        "(                                "
        "100000, '" + service::pwdEncrypt("123456") + "', 'admin', 1, 1, 0, 1);";
    res = query.exec(creatTableStr);

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
        "notice_manage    tinyint(1)  NOT NUll,"
        "PRIMARY KEY (group_id)) ENGINE=InnoDB;"
        "INSERT INTO magic_group"
        "(group_id, group_name, users_manage, attend_manage, apply_manage, applyItem_manage, group_manage, activity_manage, send_message, notice_manage)"
        "VALUES(1, '超级管理员', 1, 1, 1, 1, 1, 1, 1, 1);"
        "INSERT INTO magic_group"
        "(group_id, group_name, users_manage, attend_manage, apply_manage, applyItem_manage, group_manage, activity_manage, send_message, notice_manage)"
        "VALUES(2, '普通用户', 0, 0, 0, 0, 0, 0, 1, 0);";
    if(res)
        res = query.exec(creatTableStr);

    //组织架构表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_department"
        "(dpt_id      int(10)      NOT NULL    AUTO_INCREMENT,"
        "dpt_name     varchar(32)  NOT NUll,"
        "PRIMARY KEY (dpt_id))ENGINE=InnoDB;"
        "INSERT INTO magic_department"
        "(dpt_id, dpt_name)"
        "VALUES(1, '默认部门');";
    if (res)
        res = query.exec(creatTableStr);

    //考勤表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_attendance"
        "(num       int(10)      NOT NULL    AUTO_INCREMENT,"
        "a_uid      int(10)      NOT NUll,"
        "begin_date datetime,"
        "end_date   datetime,"
        "today      date,"
        "isSupply   tinytext     NOT NUll,"
        "operator   int(10),"
        "PRIMARY KEY (num, a_uid)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

    //申请表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_apply"
        "(apply_id       int(10)      NOT NULL    AUTO_INCREMENT,"
        "apply_uid       int(10)      NOT NUll,"
        "alist_id        int(10)      NOT NUll,"
        "op1_text        text,"
        "op2_text        text,"
        "op3_text        text,"
        "process_uidList varchar(64),"
        "status          tinyint(1)   NOT NUll,"
        "check_uidList   varchar(64),"
        "reject_uid      int(10),"
        "PRIMARY KEY (apply_id, apply_uid)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

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
    if (res)
        res = query.exec(creatTableStr);

    //用户认证信息表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_verify"
        "(v_uid          int(10)      NOT NULL    AUTO_INCREMENT,"
        "vid             int(10)  NOT NUll,"
        "info            varchar(32),"
        "PRIMARY KEY (v_uid)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

    //用户认证类型表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_verifyList"
        "(v_id           int(10)      NOT NULL    AUTO_INCREMENT,"
        "verify_name     varchar(32)  NOT NUll,"
        "icon            tinyint(1)   NOT NUll,"
        "PRIMARY KEY (v_id)"
        ")ENGINE=InnoDB;"
        "INSERT INTO magic_verifyList"
        "(v_id, verify_name, icon)"
        "VALUES (1, '个人认证', 0);"
        "INSERT INTO magic_verifyList"
        "(v_id, verify_name, icon)"
        "VALUES (2, '机构认证', 1);";
    if (res)
        res = query.exec(creatTableStr);

    //活动表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_activity"
        "(act_id      int(10)      NOT NULL    AUTO_INCREMENT,"
        "act_name     varchar(32)  NOT NUll,"
        "act_des      text         NUll,"
        "joinDate     datetime     NOT NUll,"
        "beginDate    datetime     NOT NUll,"
        "endDate      datetime     NOT NUll,"
        "editUid      int(10)      NOT NUll,"
        "act_score    float(5,2)   NOT NUll,"
        "PRIMARY KEY (act_id)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);

    //活动成员表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_activityMembers"
        "(actm_id      int(10)      NOT NULL    AUTO_INCREMENT,"
        "act_id        int(10)      NOT NUll,"
        "actm_uid      int(10)      NOT NUll,"
        "actm_joinDate datetime     NOT NUll,"
        "status        tinytext     NOT NUll,"
        "PRIMARY KEY (actm_id)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    //通知动态表
    creatTableStr =
        "CREATE TABLE IF NOT EXISTS magic_contents"
        "(c_id         int(10)      NOT NULL    AUTO_INCREMENT,"
        "title         varchar(64)  NUll,"
        "text          mediumtext   NUll,"
        "created       datetime     NOT NUll,"
        "modified      datetime     NOT NUll,"
        "c_type        tinytext     NOT NUll,"
        "isHide        tinytext     NOT NUll,"
        "author_id     int(10)      NOT NUll,"
        "PRIMARY KEY (c_id)"
        ")ENGINE=InnoDB;";
    if (res)
        res = query.exec(creatTableStr);
    query.clear();
    
    return res;
}

int service::authAccount(QSqlDatabase& db, QString& uid, const long long account, const QString& pwd)
{
    db.open();
    QSqlQuery query(db);

    if (account == 0 || QString::number(account).isEmpty() || pwd.isEmpty())
        return 403;
    //验证UID
    bool net = query.exec("SELECT password, user_status FROM magic_users WHERE uid = " + QString::number(account));
    if (!net)
        return 500;
    if(query.next() && pwd == query.value("password").toString())
    {
        uid = QString::number(account);
        qDebug() << uid << "登录方式：账号登录";
        if (query.value("user_status").toInt() == 0)    //账号状态检测
            return 400;
        return 200;
    }
    else
    {
        //验证手机号，可能有重复的手机号，所以用while
        net = query.exec("SELECT uid, password, user_status FROM magic_users WHERE telephone = " + QString::number(account));
        if (!net)
            return 500;
        while(query.next())
            if(pwd == query.value("password").toString())
            {
                uid = query.value("uid").toString();
                qDebug() << uid << "登录方式：手机号登录";
                if (query.value("user_status").toInt() == 0)    //账号状态检测
                    return 400;
                return 200;
            }
        return 403;
    }
}

bool service::setAuthority(QSqlDatabase& db, const QString &uid, const QVector<QAction*>& vector)
{
    QSqlQuery query(db);
    query.exec("SELECT user_group FROM magic_users WHERE uid='" + uid + "';");
    query.next();
    QString groupId = query.value(0).toString();
    query.exec("SELECT * FROM magic_group WHERE group_id='" + groupId + "';");

    if(query.next())
    {
        if(query.value("send_message").toString() == '0')
        {
            vector[0]->setEnabled(false);
            vector[0]->setVisible(false);
        }
        else{
            vector[0]->setEnabled(true);
            vector[0]->setVisible(true);
        }
        if(query.value("users_manage").toString() == '0')
        {
            vector[1]->setEnabled(false);
            vector[1]->setVisible(false);

        }
        else{
            vector[1]->setEnabled(true);
            vector[1]->setVisible(true);
        }
        if(query.value("attend_manage").toString() == '0')
        {
            vector[2]->setEnabled(false);
            vector[2]->setVisible(false);

        }
        else{
            vector[2]->setEnabled(true);
            vector[2]->setVisible(true);
        }
        if(query.value("activity_manage").toString() == '0')
        {
            vector[3]->setEnabled(false);
            vector[3]->setVisible(false);

        }
        else{
            vector[3]->setEnabled(true);
            vector[3]->setVisible(true);
        }
        if(query.value("apply_manage").toString() == '0')
        {
            vector[4]->setEnabled(false);
            vector[4]->setVisible(false);

        }
        else{
            vector[4]->setEnabled(true);
            vector[4]->setVisible(true);
        }
        if(query.value("applyItem_manage").toString() == '0')
        {
            vector[5]->setEnabled(false);
            vector[5]->setVisible(false);

        }
        else{
            vector[5]->setEnabled(true);
            vector[5]->setVisible(true);
        }
        if(query.value("group_manage").toString() == '0')
        {
            vector[6]->setEnabled(false);
            vector[6]->setVisible(false);

        }
        else{
            vector[6]->setEnabled(true);
            vector[6]->setVisible(true);
        }
    }else
    {
        return false;
    }
    return true;
}

QPixmap service::getAvatar(const QString& url)
{
    QUrl picUrl(url);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply = manager.get(QNetworkRequest(picUrl));
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

void service::buildAttendChart(QChartView *chartView_attend, const QWidget *parent, const QFont &font, int data_1, int data_2, int data_3, int data_4)
{
    QChart* attendChart = new QChart();
	QChart* oldChart = chartView_attend->chart();

    chartView_attend->setChart(attendChart);
    chartView_attend->setRenderHint(QPainter::Antialiasing);

    if (oldChart != nullptr)
    {
        delete oldChart;
        oldChart = nullptr;
    }

    attendChart->setTitle("工作时长统计（作息时间很不错~）");
    attendChart->setAnimationOptions(QChart::SeriesAnimations);
    QPieSeries *series = new QPieSeries(attendChart); //创建饼图序列
    series->setHoleSize(0.1);   //饼图空心大小
    //添加分块数据 6- 6-8 8-10 10+
    series->append("4h以下", data_1); //添加一个饼图分块数据,标签，数值
    series->append("4~6h", data_2);
    series->append("6~8h", data_3);
    series->append("8h以上", data_4);

	//设置每个分块的标签文字
    for(int i = 0; i < 4; i++)
    {
        QPieSlice* slice = series->slices().at(i);  //获取分块
        slice->setLabel(slice->label() + QString::asprintf(":%.0f次", slice->value()));    //设置分块的标签
        //信号与槽函数关联，鼠标落在某个分块上时，此分块弹出
        slice->setLabelFont(font);
        slice->setLabelVisible(true);
        if(slice->value() < 1)
            slice->setLabelVisible(false);  //过小就不显示label
        parent->connect(slice, SIGNAL(hovered(bool)), parent, SLOT(on_PieSliceHighlight(bool)));
    }
    //series->setLabelsVisible(true); //只影响当前的slices，必须添加完slice之后再设置
    attendChart->addSeries(series); //添加饼图序列

    if(series->slices().at(0)->percentage() > (series->slices().at(1)->percentage() + series->slices().at(2)->percentage()))
        attendChart->setTitle("工作时长统计（需要再勤奋一点哦！）");
    if(series->slices().at(3)->percentage() > (series->slices().at(1)->percentage() + series->slices().at(2)->percentage()))
        attendChart->setTitle("工作时长统计（要注意休息哦，身体最重要~）");
    attendChart->legend()->setFont(font);
    attendChart->setTitleFont(font);
    attendChart->legend()->setVisible(true); //图例
    attendChart->legend()->setAlignment(Qt::AlignBottom);
}
