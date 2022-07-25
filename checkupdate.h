#pragma once
#pragma execution_character_set("utf-8")

#ifndef CHECKUPDATE_H
#define CHECKUPDATE_H

#include <QObject>
#include <QLabel>
#include <QEventLoop>
#include <QMessageBox>
#include <QDesktopServices>
//Network
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
//JSON
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


class checkUpdate : public QObject
{

    Q_OBJECT

public:

    checkUpdate();

    ~checkUpdate();

    bool parse_UpdateJson(QLabel* label, QWidget* parent);

    QString getCurVersion();

    QString getLatestVersion();

    void homeCheckUpdate();

    bool getUpdateInfo();

    QString getErrorInfo();

    QUrl getUrl();

    QString getUpdateString();

private:

    QNetworkAccessManager *manager;		//定义网络请求对象

    QString CurVersion;	//定义当前软件的版本号

    QString LatestVersion;

    QString Version;

    QString Url;

    QString Notice;

    QString UpdateTime;

    QString ReleaseNote;

    QString errorInfo;

    QString updateStr;

signals:

	void finished();
    void homeCheckUpdateFinished(bool res);

};

#endif // CHECKUPDATE_H
