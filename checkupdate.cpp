#include "checkupdate.h"

checkUpdate::checkUpdate()
{
    CurVersion = "0.0.4";   //在此处定义软件当前版本
}

bool checkUpdate::parse_UpdateJson(QLabel* label, QWidget* parent)
{
    QNetworkRequest quest;
    QNetworkReply *reply;
    QEventLoop loop;

    quest.setUrl(QUrl("https://www.bytecho.net/software_update.json"));
    quest.setHeader(QNetworkRequest::UserAgentHeader, "RT-Thread ART");
    reply = manager.get(quest);

    //请求结束并下载完成后，退出子事件循环
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();

    QString res = reply->readAll();
    QJsonParseError err_rpt;
    QJsonDocument root_Doc = QJsonDocument::fromJson(res.toUtf8(), &err_rpt);//字符串格式化为JSON

    if(err_rpt.error != QJsonParseError::NoError)
    {
        //QMessageBox::critical(this, "检查失败", "服务器地址错误或JSON格式错误!");
        return false;
    }
    if(root_Doc.isObject())
    {
        QJsonObject  root_Obj = root_Doc.object();   //创建JSON对象
        QJsonObject PulseValue = root_Obj.value("MagicLightAssistant").toObject();

        QString Version = PulseValue.value("LatestVersion").toString();
        QString Url = PulseValue.value("Url").toString();
        QString Notice = PulseValue.value("Notice").toString();
        QString UpdateTime = PulseValue.value("UpdateTime").toString();
        QString ReleaseNote = PulseValue.value("ReleaseNote").toString();

        label->setText(Notice);
        qDebug() << Version;
        if(Version > CurVersion)
        {
            QString warningStr =  "检测到新版本：\n版本号：" + Version + "\n" + "发布时间：" + UpdateTime + "\n" + "更新说明：" + ReleaseNote;
            int ret = QMessageBox::warning(parent, "检查更新",  warningStr, "前往下载", "暂不更新");
            if(ret == 0)
                QDesktopServices::openUrl(QUrl(Url));
        }
    }
    return true;
}
