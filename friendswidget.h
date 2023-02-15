#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QFontDatabase>
#include <QMessageBox>
#include <QKeyEvent>
#include "ui_friendswidget.h"
#include "msgservice.h"
#include "service.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FriendsWidgetClass; };
QT_END_NAMESPACE

class FriendsWidget : public QWidget
{
	Q_OBJECT

public:
	FriendsWidget(QWidget *parent = nullptr);
	~FriendsWidget();

protected:
	void keyPressEvent(QKeyEvent* event);	//ÊÂ¼þ¹ýÂËÆ÷

private:
	QString uid = "-1", searchUid = "-1", applyUid = "-1";
	QThread* thread;
	MsgService* msgService;
	void loading();
	void setMemberInfo(QByteArray array);

private slots:
	void on_btn_search_clicked();
	void on_btn_sendApply_clicked();
	void on_btn_agree_clicked();
	void on_btn_reject_clicked();
signals:
	void searchMember(const QString& uid);
	void loadApplyInfo(const QString& me, const QString& member);
	void sendApply(const QString& me, const QString& member, const QString& info);
	void operateApply(const QString& me, const QString& member, int flag);
	void loadApplyInfoFinished();

public:
	void initInfo();
	void setUid(const QString& uid);
	void loadApply(const QString& uid);
	void set_group_applyInfo_enabled(bool flag);
	void set_group_addFriends_enabled(bool flag);
private:
	Ui::FriendsWidgetClass* ui;
};
