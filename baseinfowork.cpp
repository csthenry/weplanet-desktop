#include "baseinfowork.h"

baseInfoWork::baseInfoWork(QObject *parent) : QObject(parent)
{

}

void baseInfoWork::loadBaseInfoWorking(QString uid)
{
    QSqlQuery query;
    QThread::msleep(50);    //等待GUI相应的时间，以免程序卡顿或崩溃
    query.exec("SELECT name, gender, telephone, mail, user_group, user_dpt, user_avatar FROM magic_users WHERE uid = " + uid);
    if(query.next())
    {
        name = query.value("name").toString();
        gender = query.value("gender").toString();
        telephone = query.value("telephone").toString();
        mail = query.value("mail").toString();
        avatarUrl = query.value("user_avatar").toString();
    }
    avatar = loadAvatar(avatarUrl);
    emit baseInfoFinished();
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
