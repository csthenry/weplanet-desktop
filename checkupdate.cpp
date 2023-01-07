#include "checkupdate.h"
#include <QMessageBox>

checkUpdate::checkUpdate()
{
    CurVersion = "0.1.8-RC-c";   //在此处定义软件当前版本
}
checkUpdate::~checkUpdate()
{
}
bool checkUpdate::parse_UpdateJson(QLabel* label, QWidget* parent)
{
    if (!getUpdateInfo())
        return false;
    else
    {
        label->setText(Notice);
        emit finished();
    }
    return true;
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
        QJsonObject PulseValue = root_Obj.value("MagicLightAssistant").toObject();

        Version = PulseValue.value("LatestVersion").toString();
        Url = PulseValue.value("Url").toString();
        Notice = PulseValue.value("Notice").toString();
        UpdateTime = PulseValue.value("UpdateTime").toString();
        ReleaseNote = PulseValue.value("ReleaseNote").toString();
        qDebug() << Version;
        LatestVersion = Version;

        return true;
    }
    return false;
}

QString checkUpdate::getErrorInfo()
{
    return errorInfo;
}

QUrl checkUpdate::getUrl()
{
    return QUrl(Url);
}

QString checkUpdate::getUpdateString()
{
    return updateStr;
}


QString checkUpdate::getCurVersion()
{
    return CurVersion;
}

QString checkUpdate::getLatestVersion()
{
    return LatestVersion;
}
