#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QFontDatabase>
#include <QMessageBox>
#include <QClipboard>
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
	QString m_uid;	//µ±«∞∫√”—UID
	QThread* thread;
	baseInfoWork* infoWork;
	void setLoading();
	void setInfoPage();
	QClipboard *clipboard;
public:
	void setUid(const QString& uid);
	QString getUid();
public:
	FriendInfoWidget(QWidget *parent = nullptr);
	~FriendInfoWidget();
private slots:
	void on_btn_share_clicked();
signals:
	void getCurrentMemInfo();
private:
	Ui::FriendInfoWidgetClass *ui;
};
