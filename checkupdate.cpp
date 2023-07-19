#include "checkupdate.h"
#include <QMessageBox>

checkUpdate::checkUpdate()
{
    CurVersion = "1.3.0.4";   //在此处定义软件当前版本
    AutoUpdateToolVersion = 11304;   //自动更新工具识别版本
    writeVersion();
}
checkUpdate::~checkUpdate()
{
}
bool checkUpdate::parse_UpdateJson()
{
    if (!getUpdateInfo())
        return false;
    else
        emit finished(Notice);
    return true;
}

void checkUpdate::writeVersion()   //用于更新工具识别版本
{
    QString configFilePath = QDir::currentPath() + "/update/version.dat";
    configFilePath = QDir::toNativeSeparators(configFilePath);
    QFile writeFile(configFilePath);
    writeFile.open(QIODevice::WriteOnly);
    QDataStream out(&writeFile);
	out << AutoUpdateToolVersion;   //写入版本号
    writeFile.close();
}

void checkUpdate::homeCheckUpdate()
{
    if (getUpdateInfo())
    {
        if (Version > CurVersion)
            updateStr = "检测到新版本：\n软件版本号：" + Version + "\n" + "发布时间：" + UpdateTime + "\n" + "更新说明：" + ReleaseNote;
        emit homeCheckUpdateFinished(true);
    }
    else 
        emit homeCheckUpdateFinished(false);
}

bool checkUpdate::getUpdateInfo()
{
    QNetworkRequest quest;
    QNetworkReply* reply;
    QEventLoop loop;
    quest.setUrl(QUrl("https://www.bytecho.net/software_update.json"));
    quest.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.82 Safari/537.36");
    manager = new QNetworkAccessManager();
    reply = manager->get(quest);

    //请求结束并下载完成后，退出子事件循环
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();
    QString res = reply->readAll();
    QJsonParseError err_rpt;
    QJsonDocument root_Doc = QJsonDocument::fromJson(res.toUtf8(), &err_rpt);//字符串格式化为JSON

    if (err_rpt.error != QJsonParseError::NoError)
    {
        errorInfo = reply->errorString();
        return false;
    }
    delete manager;
    if (root_Doc.isObject())
    {
        QJsonObject  root_Obj = root_Doc.object();   //创建JSON对象
        QJsonObject PulseValue = root_Obj.value("WePlanet").toObject();

		isForce = PulseValue.value("isForce").toBool();
        Version = PulseValue.value("LatestVersion").toString();
        //Url = PulseValue.value("Url").toString(); 已废弃
        Notice = PulseValue.value("Notice").toString();
        UpdateTime = PulseValue.value("UpdateTime").toString();
        ReleaseNote = PulseValue.value("ReleaseNote").toString();
        qDebug() << "[Update]当前软件版本：" << CurVersion;
        qDebug() << "[Update]服务器版本：" << Version;
        LatestVersion = Version;

        return true;
    }
    return false;
}

QString checkUpdate::getErrorInfo()
{
    return errorInfo;
}

//QUrl checkUpdate::getUrl()
//{
//    return QUrl(Url);
//}

QString checkUpdate::getUpdateString()
{
    return updateStr;
}

bool checkUpdate::getIsForce()
{
    return isForce;
}


QString checkUpdate::getCurVersion()
{
    return CurVersion;
}

QString checkUpdate::getLatestVersion()
{
    return LatestVersion;
}
