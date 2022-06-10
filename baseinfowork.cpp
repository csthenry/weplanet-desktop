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
    query.exec("SELECT name, gender, telephone, mail, user_group, user_dpt, user_avatar, score FROM magic_users WHERE uid = " + uid);
    if(query.next())
    {
        name = query.value("name").toString();
        gender = query.value("gender").toString();
        telephone = query.value("telephone").toString();
        mail = query.value("mail").toString();
        avatarUrl = query.value("user_avatar").toString();
        score = query.value("score").toString();
    }
    group = loadGroup(uid);
    department = loadDepartment(uid);
    avatar = loadAvatar(avatarUrl);

    query.exec("SELECT * FROM magic_attendance WHERE a_uid='" + uid + "' AND today='" + curDateTime.date().toString("yyyy-MM-dd") + "';");
    if(query.next())
    {
        isAttend = true;
        attendBeginTime = query.value("begin_date").toString();
        attendEndTime = query.value("end_date").toString();
    }
    else
        isAttend = false;
    query.clear();
    DB.close();
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

void baseInfoWork::bindQQAvatar(QString qqMail)
{
    DB.open();
    QSqlQuery query(DB);
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
            qDebug() << "这是请求到的头像地址Header" << res;
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

            query.exec("UPDATE magic_users SET user_avatar='" + avatarUrl + "' WHERE uid='" + uid + "';");
            if (!query.lastError().text().isEmpty())
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
    return res;
}

QString baseInfoWork::loadDepartment(const QString& uid)
{
    QString res;
    QSqlQuery query(DB);
    query.exec("SELECT user_dpt FROM magic_users WHERE uid = " + uid);
    if(!query.next())
        return "--";
    query.exec("SELECT dpt_name FROM magic_department WHERE dpt_id = " + query.value("user_dpt").toString());
    if(!query.next())
        return "--";
    res = query.value("dpt_name").toString();
    query.clear();
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
    QSqlQuery query(DB);
    QString creatQueryStr;
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
    emit signupRes(query.exec());
    lastSignupUid = query.lastInsertId().toString();
    query.clear();
    DB.close();
}

void baseInfoWork::editPersonalInfo(const QString& oldPwd, const QString &tel, const QString &mail, const QString &avatar, const QString &pwd)
{
    DB.open();
    QSqlQuery query(DB);
    if(service::authAccount(DB, uid, uid.toLongLong(), service::pwdEncrypt(oldPwd)))
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
    int res = service::authAccount(DB, loginUid, account, pwd);
    if(res == 403)
    {
        res = service::authAccount(DB, loginUid, account, editPwd);
    }
    emit authRes(res);
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
