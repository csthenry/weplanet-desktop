#include "baseinfowork.h"

baseInfoWork::baseInfoWork(QObject *parent) : QObject(parent)
{
    db_service.addDatabase(DB, "baseInfoWork_DB");
    db_service.addDatabase(initDB, "baseInfoWork_initDB");
}

void baseInfoWork::loadBaseInfoWorking()
{
    DB.open();
    curDateTime = QDateTime::currentDateTime();
    this->uid = uid;
    QSqlQuery query(DB);
    QThread::msleep(30);    //等待GUI相应的时间
    query.exec("SELECT * FROM magic_users WHERE uid = " + uid);
    if(query.next())
    {
        name = query.value("name").toString();
        gender = query.value("gender").toString();
        telephone = query.value("telephone").toString();
        mail = query.value("mail").toString();
        avatarUrl = query.value("user_avatar").toString();
        score = query.value("score").toString();
        lastLoginTime = query.value("last_login").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
    }
    else
    {
        name = "--";
        gender = "--";
		telephone = "--";
		mail = "--";
		avatarUrl = "";
		score = "--";
		lastLoginTime = "--";
    }

    query.exec("SELECT * FROM magic_attendance WHERE a_uid='" + uid + "' AND today='" + curDateTime.date().toString("yyyy-MM-dd") + "';");
    if(query.next())
    {
        isAttend = true;
        attendBeginTime = query.value("begin_date").toString();
        attendEndTime = query.value("end_date").toString();
    }
    else
        isAttend = false;

    query.exec("SELECT * FROM magic_verify WHERE v_uid = " + uid);
    if (query.next())
    {
		verifyTag = query.value("vid").toInt();
		verifyInfo = query.value("info").toString();
		
        query.exec("SELECT * FROM magic_verifyList WHERE v_id = " + QString::number(verifyTag));
        query.next();
        verifyType = query.value("verify_name").toString();
    }
    else
    {
		verifyTag = -1;
		verifyType = "";
        verifyInfo = "";
    }
    if (smtp_config.isEmpty() || smtp_isNewConfig)
    {
        query.exec("SELECT * FROM magic_system WHERE sys_name='smtp';");
        if (query.next() && !query.value("field_1").toString().isEmpty())
        {
            smtp_config.clear();
            smtp_config.push_back(query.value("field_1").toString());
            smtp_config.push_back(query.value("field_2").toString());
            smtp_config.push_back(query.value("field_3").toString());
        }
    }
    query.clear();
    DB.close();

    //此处操作会关闭数据库连接，故最后执行
    group = loadGroup(uid);
    department = loadDepartment(uid);
    avatar = loadAvatar(avatarUrl);

    emit baseInfoFinished();
    qDebug() << "baseInfoLoad线程：" << this->thread();
}

void baseInfoWork::refreshBaseInfo()
{
    loadBaseInfoWorking();
}

void baseInfoWork::setUid(QString uid)
{
    this->uid = uid;
}

void baseInfoWork::initDatabaseTables()
{
    initDB.open();
	emit initDatabaseFinished(service::initDatabaseTables(initDB));
    initDB.close();
}

bool baseInfoWork::getAttendToday()
{
    return isAttend;
}

bool baseInfoWork::getSys_isOpenChat()
{
    return sys_openChat;
}

void baseInfoWork::bindQQAvatar(QString qqMail)
{
    int pos = 0;
    QNetworkRequest quest;
    QNetworkReply* reply;
    QNetworkAccessManager manager;
    QEventLoop loop;

    QString qqNum, avatarSdk, avatar_api = "https://ptlogin2.qq.com/getface?&imgtype=1&uin=", avatarUrl = "https://thirdqq.qlogo.cn/g?b=sdk&s=100&t=0&k=";
    QRegExp qqMailExp("[0-9]{5,11}@qq+\\.com"); //qq邮箱正则
    QRegExpValidator qqMailExpValidator(qqMailExp);
    if (qqMailExpValidator.validate(qqMail, pos) == QValidator::Acceptable)
    {
        qqNum = qqMail.mid(0, qqMail.indexOf("@"));
        if (!qqNum.isEmpty())
        {
            avatar_api += qqNum;
            quest.setUrl(QUrl(avatar_api));     //调用api获取QQ头像SDK
            quest.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.82 Safari/537.36");
            reply = manager.get(quest);

            //请求结束并下载完成后，退出子事件循环
            QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            //开启子事件循环
            loop.exec();

            QString res = reply->readAll();
            qDebug() << "请求到的头像地址Header" << res;
            if (res.isEmpty())
            {
                qDebug() << "QQ头像地址获取失败：服务器错误!\n错误信息：" + reply->errorString();
                emit bindQQAvatarFinished(-1);
                return;
            }
            int leftIdx = res.indexOf(qqNum) + qqNum.length() + 3, rightIdx = res.lastIndexOf("&s");
            avatarSdk = res.mid(leftIdx, rightIdx - leftIdx);
            avatarSdk = avatarSdk.mid(avatarSdk.indexOf("&k=") + 3, -1);
            avatarUrl += avatarSdk;
            qDebug() << "头像SDK" << avatarSdk << " 头像地址" + avatarUrl;

            DB.open();
            QSqlQuery query(DB);
            bool query_res = query.exec("UPDATE magic_users SET user_avatar='" + avatarUrl + "' WHERE uid='" + uid + "';");
            query.clear();
            DB.close();

            if (!query_res)
            {
                emit bindQQAvatarFinished(-1);
                return;
            }
        }
        else
        {
            emit bindQQAvatarFinished(-1);
            return;
        }
    }
    else
    {
        emit bindQQAvatarFinished(0);   //不是QQ邮箱
        return;
    }
    emit bindQQAvatarFinished(1);
}

void baseInfoWork::bindMailAvatar(QString mail)
{
    mail.trimmed(); //去空格
    QString hash = service::pwdEncrypt(mail);
    QString avatarUrl = QString("https://cravatar.cn/avatar/%1?s=120&r=G&d=mp").arg(hash);
    DB.open();
    QSqlQuery query(DB);
    bool query_res = query.exec("UPDATE magic_users SET user_avatar='" + avatarUrl + "' WHERE uid='" + uid + "';");
    query.clear();
    DB.close();

    emit bindMailAvatarFinished(query_res);
}

QString baseInfoWork::getBeginTime()
{
    return attendBeginTime;
}

QString baseInfoWork::getEndTime()
{
    return attendEndTime;
}

QString baseInfoWork::getLoginUid()
{
    return loginUid;
}

QString baseInfoWork::getLastSignupUid()
{
    return lastSignupUid;
}

QString baseInfoWork::loadGroup(const QString& uid)
{
    DB.open();
    QString res;
    QSqlQuery query(DB);
    query.exec("SELECT user_group FROM magic_users WHERE uid = " + uid);
    if(!query.next())
        return "--";
    query.exec("SELECT group_name FROM magic_group WHERE group_id = " + query.value("user_group").toString());
    if(!query.next())
        return "--";
    res = query.value("group_name").toString();
    query.clear();
    DB.close();
    return res;
}

QString baseInfoWork::loadDepartment(const QString& uid)
{
    QString res;
    DB.open();
    QSqlQuery query(DB);
    query.exec("SELECT user_dpt FROM magic_users WHERE uid = " + uid);
    if(!query.next())
        return "--";
    query.exec("SELECT dpt_name FROM magic_department WHERE dpt_id = " + query.value("user_dpt").toString());
    if(!query.next())
        return "--";
    res = query.value("dpt_name").toString();
    query.clear();
    DB.close();
    return res;
}

void baseInfoWork::autoAuthAccount(const long long account, const QString &pwd)
{
    DB.open();
    emit autoAuthRes(service::authAccount(DB, loginUid, account, pwd));
    DB.close();
}

void baseInfoWork::signUp(const QString& pwd, const QString& name, const QString& tel, const QString& gender)
{
    DB.open();
    QSqlQuery query(DB), statistics(DB);
    QString creatQueryStr;
	
    //统计注册请求量
    statistics.exec("SELECT * FROM magic_statistics WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    if (statistics.next())
        statistics.exec("UPDATE magic_statistics SET register_cnt=register_cnt+1 WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    else
        statistics.exec("INSERT INTO magic_statistics (date, register_cnt) VALUES ('" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "', 1)");
    statistics.clear();
    
	query.exec(QString("SELECT uid FROM magic_users WHERE telephone = '%1';").arg(tel));    //判断手机号是否已注册
	if (query.next())
	{
		emit signupRes(102);
        query.clear();
        DB.close();
		return;
	}
    creatQueryStr =
            "INSERT INTO magic_users"
            "(password, name, user_group, user_dpt, telephone, gender, score, user_status)"
            "VALUES                        "
            "(:pwd, :name, 2, 1, :phone, :gender, 0, 1) ";
    query.prepare(creatQueryStr);
    query.bindValue(0, service::pwdEncrypt(pwd));
    query.bindValue(1, name);
    query.bindValue(2, tel);
    query.bindValue(3, gender);
    if(query.exec())
        emit signupRes(100);
    else
		emit signupRes(101);
    lastSignupUid = query.lastInsertId().toString();
    query.clear();
    DB.close();
}

void baseInfoWork::editPersonalInfo(const QString& oldPwd, const QString &tel, const QString &mail, const QString &avatar, const QString &pwd)
{
    DB.open();
    QSqlQuery query(DB);
    if(service::authAccount(DB, uid, uid.toLongLong(), service::pwdEncrypt(oldPwd)) == 200)
    {
        if(!tel.isEmpty())
            query.exec("UPDATE magic_users SET telephone='" + tel +"' WHERE uid='" + uid +"';");
        if(!mail.isEmpty())
            query.exec("UPDATE magic_users SET mail='" + mail +"' WHERE uid='" + uid +"';");
        if(!avatar.isEmpty())
            query.exec("UPDATE magic_users SET user_avatar='" + avatar +"' WHERE uid='" + uid +"';");
        if(!pwd.isEmpty())
        {
            query.exec("UPDATE magic_users SET password='" + service::pwdEncrypt(pwd) +"' WHERE uid='" + uid +"';");
            emit editPersonalInfoRes(2);
        }
        else emit editPersonalInfoRes(1);
    }
    else
        emit editPersonalInfoRes(-1);
    query.clear();
    DB.close();
}

void baseInfoWork::authAccount(const long long account, const QString &pwd, const QString& editPwd)
{
    DB.open();
    int res = service::authAccount(DB, loginUid, account, pwd);
    if(res == 403)
        res = service::authAccount(DB, loginUid, account, editPwd);
    emit authRes(res);
    DB.close();
}

void baseInfoWork::setAuthority(const QString &uid)
{
    DB.open();
    qDebug() << "校验用户权限...";
    QSqlQuery query(DB);
    QSqlRecord res;
    query.exec("SELECT user_group FROM magic_users WHERE uid='" + uid + "';");
    query.next();
    QString groupId = query.value(0).toString();
    query.exec("SELECT * FROM magic_group WHERE group_id='" + groupId + "';");
    query.next();
    res = query.record();
    DB.close();
    emit authorityRes(res);
}

void baseInfoWork::updateScore(float score)
{
    DB.open();
    qDebug() << "正在将已完成活动学时写入数据库...";
    QSqlQuery query(DB);
    query.exec("UPDATE magic_users SET score ='" + QString::number(score) + "' + score WHERE uid = '" + uid + "'");
    query.clear();
    DB.close();
}

void baseInfoWork::get_statistics()
{
    DB.open();
    QSqlQuery statistics(DB);

    //统计心跳请求量
    statistics.exec("SELECT * FROM magic_statistics WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    if (statistics.next())
        statistics.exec("UPDATE magic_statistics SET get_cnt=get_cnt+1 WHERE date='" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "'");
    else
        statistics.exec("INSERT INTO magic_statistics (date, get_cnt) VALUES ('" + QDateTime::currentDateTime().date().toString("yyyy-MM-dd") + "', 1)");
    statistics.clear();
    DB.close();
}

void baseInfoWork::getAnnouncement()
{
	DB.open();
	QSqlQuery query(DB);
	query.exec("SELECT * FROM magic_system WHERE sys_name='announcement'");
    bool res = query.next();
    if (res)
    {
        announcementTag = query.value("field_1").toInt();
        announcementText = query.value("field_2").toString();
		res = query.value("field_3").toBool();
    }
    query.exec("SELECT * FROM magic_system WHERE sys_name='debug'"); 
    if (query.next())
        isDebug = query.value("field_1").toBool();
    qDebug() << "system debug:" << query.value("field_1").toBool();
	query.clear();
	DB.close();

	emit getAnnouncementFinished(res);
}

void baseInfoWork::loadStatisticsPanel()
{
    DB.open();
	QSqlQuery query(DB);
	QDateTime date = QDateTime::currentDateTime();
    QJsonArray login_cnt, register_cnt, get_cnt, activity_cnt, dynamices_cnt;   //14天数据
    QJsonArray login_cnt_half, register_cnt_half, get_cnt_half, activity_cnt_half, dynamices_cnt_half;  //7天数据
    date = date.addDays(-14);
    for (int i = 0; i < 14; i++)
    {
        date = date.addDays(1);
        query.exec("SELECT * FROM magic_statistics WHERE date='" + date.date().toString("yyyy-MM-dd") + "'");
        if (query.next())
        {
            QSqlRecord record = query.record();
			login_cnt.append(record.value("login_cnt").toInt());
			register_cnt.append(record.value("register_cnt").toInt());
			get_cnt.append(record.value("get_cnt").toInt());
			activity_cnt.append(record.value("activity_cnt").toInt());
			dynamices_cnt.append(record.value("dynamics_cnt").toInt());
        }
        else
        {   //当天没有记录，设初值0
            login_cnt.append(0);
			register_cnt.append(0);
			get_cnt.append(0);
			activity_cnt.append(0);
			dynamices_cnt.append(0);
        }
    }
    for (int i = 7; i < 14; i++)
    {
        login_cnt_half.append(login_cnt[i]);
		register_cnt_half.append(register_cnt[i]);
		get_cnt_half.append(get_cnt[i]);
		activity_cnt_half.append(activity_cnt[i]);
		dynamices_cnt_half.append(dynamices_cnt[i]);
    }
	//构建json obj
    panelSeriesObj.insert("data_login", login_cnt);
	panelSeriesObj.insert("data_register", register_cnt);
	panelSeriesObj.insert("data_get", get_cnt);
	panelSeriesObj.insert("data_activity", activity_cnt);
	panelSeriesObj.insert("data_dynamices", dynamices_cnt);

    panelSeriesObj_half.insert("data_login", login_cnt_half);
	panelSeriesObj_half.insert("data_register", register_cnt_half);
	panelSeriesObj_half.insert("data_get", get_cnt_half);
	panelSeriesObj_half.insert("data_activity", activity_cnt_half);
	panelSeriesObj_half.insert("data_dynamices", dynamices_cnt_half);
	
	query.clear();
    DB.close();
    emit loadStatisticsPanelFinished(0, -1);
}

void baseInfoWork::loadSystemSettings()
{
    bool res = false;
    DB.open();
    QSqlQuery query(DB);
    query.exec("SELECT * FROM magic_system WHERE sys_name='announcement'");
    res = query.next();
	if (res)
	{
		QSqlRecord record = query.record();
		sys_isAnnounceOpen = record.value("field_3").toBool();
        sys_isTipsAnnounce = record.value("field_1").toBool();
        sys_announcementText = record.value("field_2").toString();
	}
    query.exec("SELECT * FROM magic_system WHERE sys_name='debug'");
    res = query.next();
    if (res)
    {
        QSqlRecord record = query.record();
        sys_isDebugOpen = record.value("field_1").toBool();
    }
    query.exec("SELECT * FROM magic_system WHERE sys_name='openChat'");
    res = query.next();
    if (res)
    {
        QSqlRecord record = query.record();
        sys_openChat = record.value("field_1").toBool();
    }
    query.clear();
    DB.close();
    emit loadSystemSettingsFinished(res);
}

void baseInfoWork::saveSystemSettings()
{
    bool res;
    DB.open();
    QSqlQuery query(DB);
	query.exec("UPDATE magic_system SET field_1='" + QString::number(sys_isDebugOpen) + "' WHERE sys_name='debug'");
    query.exec("UPDATE magic_system SET field_1='" + QString::number(sys_openChat) + "' WHERE sys_name='openChat'");
	res = query.exec("UPDATE magic_system SET field_1='" + QString::number(sys_isTipsAnnounce) + "', field_2='" + sys_announcementText + "', field_3='" + QString::number(sys_isAnnounceOpen) + "' WHERE sys_name='announcement'");
	query.clear();
	DB.close();
	emit saveSystemSettingsFinished(res);
}

void baseInfoWork::saveSmtpSettings(const QString& add, const QString& user, const QString& password)
{
    bool res;
    DB.open();
    QSqlQuery query(DB);
    res = query.exec(QString("UPDATE magic_system SET field_1='%1', field_2='%2', field_3='%3' WHERE sys_name='smtp'").arg(add, user, password));
    query.clear();
    DB.close();
    emit saveSmtpSettingsFinished(res);
}

QJsonObject baseInfoWork::getPanelSeriesObj(int type)
{
    return type == 1 ? panelSeriesObj : panelSeriesObj_half;
}

QString baseInfoWork::getName()
{
    return name;
}

QString baseInfoWork::getGender()
{
    return gender;
}

QString baseInfoWork::getTel()
{
    return telephone;
}

QString baseInfoWork::getMail()
{
    return mail;
}

QString baseInfoWork::getGroup()
{
    return group;
}

QString baseInfoWork::getDepartment()
{
    return department;
}

QString baseInfoWork::getScore()
{
    return score;
}

QPixmap baseInfoWork::getAvatar()
{
    return avatar;
}

QString baseInfoWork::getVerifyType()
{
    return verifyType;
}

QString baseInfoWork::getVerifyInfo()
{
    return verifyInfo;
}

QString baseInfoWork::getLastLoginTime()
{
    return lastLoginTime;
}

QString baseInfoWork::getAnnouncementText()
{
    return announcementText;
}

int baseInfoWork::getVerifyTag()
{
    return verifyTag;
}

int baseInfoWork::getAnnouncementTag()
{
    return announcementTag;
}

bool baseInfoWork::getIsDebug()
{
    return isDebug;
}

bool baseInfoWork::getSys_isAnnounceOpen()
{
    return sys_isAnnounceOpen;
}

bool baseInfoWork::getSys_isTipsAnnounce()
{
    return sys_isTipsAnnounce;
}

bool baseInfoWork::getSys_isDebugOpen()
{
    return sys_isDebugOpen;
}

void baseInfoWork::setSys_isAnnounceOpen(bool arg)
{
    sys_isAnnounceOpen = arg;
}

void baseInfoWork::setSys_isTipsAnnounce(bool arg)
{
    sys_isTipsAnnounce = arg;
}

void baseInfoWork::setSys_isDebugOpen(bool arg)
{
    sys_isDebugOpen = arg;
}

void baseInfoWork::setSys_announcementText(const QString& arg)
{
    sys_announcementText = arg;
}

void baseInfoWork::setSys_openChat(bool arg)
{
    sys_openChat = arg;
}

void baseInfoWork::setSmtp_isNewConfig(bool arg)
{
    smtp_isNewConfig = true;
}

QString baseInfoWork::getSys_announcementText()
{
    return sys_announcementText;
}

QList<QString> baseInfoWork::getSmtpConfig()
{
    return smtp_config;
}

QPixmap baseInfoWork::loadAvatar(const QString &url)
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
