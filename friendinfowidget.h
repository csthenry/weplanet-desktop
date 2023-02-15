#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QFontDatabase>
#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>
#include "baseinfowork.h"
#include "ui_friendinfowidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FriendInfoWidgetClass; };
QT_END_NAMESPACE

class FriendInfoWidget : public QWidget
{
	Q_OBJECT

private:
	bool isLoading = false;
	QString m_uid, m_mail;	//当前好友UID、邮箱
	QString fromUserInfo;	//发送者信息
	QThread* thread;
	baseInfoWork* infoWork;
	void setLoading();
	void setInfoPage();
	QClipboard *clipboard;
	QTimer* aeMovieTimer;
	bool Mailsending = false;
	QList<QString> smtp_config;
public:
	void setUid(const QString& uid);
	void setTitle(const QString& title);
	void setFromUserInfo(const QString& fromUserInfo);
	void setSmtpConfig(const QList<QString> smtp_config);
	void hideButton(bool isHide);
	QString getMail();
	QString getUid();

protected:
	void keyPressEvent(QKeyEvent* event);	//事件过滤器

public:
	FriendInfoWidget(QWidget *parent = nullptr);
	~FriendInfoWidget();
private slots:
	void on_btn_share_clicked();
	void on_btn_sendMsg_clicked();
signals:
	void getCurrentMemInfo();
private:
	Ui::FriendInfoWidgetClass *ui;
};
