/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/
#pragma once
#pragma execution_character_set("utf-8")


#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define AUTO_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" //自启注册表地址

#include <QMainWindow>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QToolButton>
#include <QPixmap>
#include <QDateTime>
#include <QLabel>
#include <QMovie>
#include <QMenu>
#include <QScrollBar>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QWebChannel>
#include <QClipboard>
#include <QShortcut>
#include "service.h"
#include "formlogin.h"
#include "querymodel.h"
#include "comboboxdelegate.h"
#include "excelexport.h"
#include "readOnlyDelegate.h"
#include "checkupdate.h"
#include "sqlwork.h"
#include "baseinfowork.h"
#include "attendwork.h"
#include "usermanagework.h"
#include "attendmanagework.h"
#include "groupmanagework.h"
#include "activitymanagework.h"
#include "document.h"
#include "previewpage.h"
#include "posterwork.h"
#include "infowidget.h"
#include "msgservice.h"
#include "friendswidget.h"
#include "friendinfowidget.h"
#include "approvalwork.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    
    QFont HarmonyOS_Font;

    QString HarmonyOS_Font_Family;

    PreviewPage* notice_page;

    QWebChannel* c_channel, *m_channel;

    Document m_content, c_content;

    QTimer *refTimer, *msgPushTimer, *currentTimeUpdate, *aeMovieTimer;

    QShortcut* shortcut;

    bool homeLoading = false, settingLoading = false, msgSending = false, personalSubmitting, qqAvatarBinding = false;   //用于加载动画判断

    bool dbStatus = true;

    bool openChat = false;  //是否开启聊天

    bool isSending = false; //消息发送中

    bool isPushing = false; //消息推送中

    int msgBeforePos = 0;   //消息记录光标位置

    int curMsgStackCnt = 0; //当前消息栈数据量

    int msgStackMax = 30; //聊天记录最大数量

    QThread *dbThread, *sqlThread, *sqlThread_MSG, *sqlThread_MSGPUSHER, *sqlThread_SECOND;

    QString uid, removedGroupId, removedDptId, sendToUid = "-1";

    QString msg_contents, msgHistoryInfo; //聊天记录、当前聊天信息

    QQueue<QString> getVerifyQueue, getAvatarQueue;

    QSystemTrayIcon* trayIcon;

    QMenu *trayIconMenu;

    QAction* mShowMainAction, *mExitAppAction, *mShowExitAction;

    QDateTime curDateTime;

    QVector<QAction*> actionList;

    QSqlDatabase db;

    QStringList comboxList;

    QLabel *connectStatusLable, *statusIcon;

    QSqlTableModel *groupModel, *departmentModel, *activityModel, *activityMemModel, *noticeModel, *noticeManageModel;  //数据模型

    QSqlRelationalTableModel *userManageModel, *attendUserModel, *attendManageModel, *attendPageModel;

    QDataWidgetMapper* actEditMapper, *noticeEditMapper;

    QItemSelectionModel *groupPageSelection_group, *groupPageSelection_department, *userManagePageSelection, *activitySelection, *activityMemSelection, *myActListSelection, *myActSelection, *noticeManageSelection, *noticeSelection, *attendUserSelection; //选择模型

    queryModel *relTableModel, *relTableModel_attend;

    QDataWidgetMapper *userManagePage_dataMapper; //数据映射

    QPixmap *statusOKIcon, *statusErrorIcon, *verifyIcon_1, *verifyIcon_2, *verifyNone, *userAvatar;

    formLogin *formLoginWindow;

    readOnlyDelegate *readOnlyDelegate;

    ComboBoxDelegate comboxDelegateAuthority, comboxDelegateGender, comboxDelegateUserGroup, comboxDelegateUserDpt, comboxDelegateUserVerify;   //自定义数据代理

    QString eChartsJsCode;

    QJsonObject panel_seriesObj, panel_display;     //智慧大屏

    QList<QToolButton*> msgMemberList, msgMemApplyList;  //好友列表

    QToolButton* msgApplyMemBtn;

    QString msgApplyMemBtn_Text;

    QLabel* msgListTips_1, *msgListTips_2;

    QString active_id = -1; //活动管理页面当前选择的ID

	QHash<QString, QString> applyItemsOptions, applyItemsAuditorList, applyItemsIsHide; //申请表单、审核人、是否隐藏

	QList<QToolButton*> manageApplyItemProcess, applyItemProcess; //申请表单审核进度

	QList<QTextEdit*> applyItemOptions_textEdit, applyItemOptions_manage_textEdit; //申请表单填写框

    QList<QToolButton*> manageApplyAuditorList; //审批人员总列表

	QList<QString> currentApplyItemAuditorList, currentApplyItemOptions;    //当前编辑的申请表单的审核人员、申请表单填写项
    
    QList<QLabel*> manage_processArrow, user_processArrow;  //用于流程图的指示箭头
    
	QString currentApplyItemID_user, currentApplyItemID_manage, currentApplyFormID_user, currentApplyFormID_manage; //当前编辑的申请项目的ID，当前申请表ID

    QString currentApplyFormUid;    //当前审核申请表的申请人UID

	QByteArray newApplyItem; //新增的申请项（标题->选项->发布者->流程->isHide）
    
	bool isApplyItemEdit = false;   //是否正在编辑申请表单

    int msgListTipsType = -1;
	
	int panel_series_count = 14, panel_option = -1;

    int msgPushTime = 5;

    void updateManageApplyItemProcess(QList<QString> list);

    void updateApplyItemProcess(int type, QString apply_id, QList<QString> list);   //0审批流程 1审批进度
    
    void updateApplyItemProcess(QList<QString> list);

	void updateApplyItemOptions(int type, QList<QString> list);//0用户页面，1审核页面

    void setProcessAutoRun(const QString& appPath, bool flag = false);  //自启函数

    bool isAutoRun(const QString& appPath); //是否开机自启

    void setHomePageBaseInfo();

    void setAttendPage();

    void setUserManagePage() const;

    void setAttendManagePage() const;

    void setGroupManagePage();

    void setActivityManagePage();

    void setNoticeManagePage();

    void setNoticePage();

    void setActivityPage();

    void setUsersFilter_group(int type, QComboBox* group, QComboBox* department);

    void setUsersFilter_dpt(int type, QComboBox* group, QComboBox* department) const;

    void reloadModelBefore();   //用于relationModel跨线程刷新，一般model似乎线程安全！

    void resetUID();    //用于切换用户后重新设置Work对象中的uid

    void initToolbar(QSqlRecord rec);

    void createActions();

    void closeEvent(QCloseEvent* event);    //重写void closeEvent(QCloseEvent *event);

    void updateFinished(QString res);

    void setStatisticsPanel(int option = 0, int days = -1);
	
    void setSystemSettings();

    void setMsgPage();

    void setApplyItemsManagePage();

    void setApplyItemsUserPage();

    void setApplyListManagePage();

    void msgPusher(QStack<QByteArray> msgStack);

    void initMsgSys();

    bool checkLocalTime();

    void disableDynamicItems(); //本地时间误差过大，禁用需要准确时间的项目

    SqlWork *sqlWork;

    baseInfoWork *setBaseInfoWork;

    AttendWork *attendWork;

    UserManageWork *userManageWork;

    AttendManageWork *attendManageWork;

    GroupManageWork *groupManageWork;

    ActivityManageWork* activityManageWork;

    PosterWork* posterWork;

    checkUpdate updateSoftWare;

    InfoWidget* infoWidget;

    FriendsWidget* friendsWidget;
    
	FriendInfoWidget* friendInfoWidget;

    MsgService* msgService, *msgPusherService;

	ApprovalWork* approvalWork;

    QSettings* config_ini;

protected:
    bool eventFilter(QObject* target, QEvent* event);//事件过滤器

public:
    MainWindow(QWidget *parent = nullptr, QDialog *formLoginWindow = nullptr);
    ~MainWindow();


private slots:

    void receiveData(QString uid); //接收登录窗口信号

    void on_actExit_triggered();

    void on_actHome_triggered();

    void on_actMyInfo_triggered();

    void on_actAttend_triggered();

    void on_actApply_triggered() ;

    void on_actUserManager_triggered();

    void on_actAttendManager_triggered();

    void on_actApplyList_triggered() ;

    void on_actApplyItems_triggered() ;

    void on_actGroup_triggered();

    void on_actMore_triggered() ;

    void on_actPanel_triggered();

    void on_actRefresh_triggered();

    void on_actSettings_triggered();

    void on_SystemTrayIconClicked(QSystemTrayIcon::ActivationReason action);

    void on_groupPageDptcurrentChanged(const QModelIndex &current, const QModelIndex &previous) const;

    void on_groupPageGroupcurrentChanged(const QModelIndex &current, const QModelIndex &previous) const;

    void on_userManagePagecurrentChanged(const QModelIndex &current, const QModelIndex &previous) const;

    void on_userManagePagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_attendManagePageUserscurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_activityPagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void on_activityManagePageMemcurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void on_myActivityPagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void on_activityManagePagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_comboBox_activity_currentIndexChanged(const QString& arg1);

    void on_comboBox_myAct_currentIndexChanged(const QString& arg1);

    void on_noticeManagePagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void on_myNoticePagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);

    void on_btn_addGroup_clicked();

    void on_btn_actApprove_clicked();

    void on_btn_actApproveAll_clicked();

    void on_btn_actReject_clicked();

    void on_btn_actDel_clicked();

    void on_btn_actClearEdit_clicked();

    void on_btn_actUpdate_clicked();

    void on_btn_actJoin_clicked();

    void on_btn_actCancel_clicked();

    void on_btn_actSearch_clicked();

    void on_btn_actSearchClear_clicked();

    void on_btn_editGroup_check_clicked();

    void on_btn_editGroup_cancel_clicked();

    void on_btn_addDpt_clicked();

    void on_btn_delDpt_clicked();

    void on_btn_delGroup_clicked();

    void on_btn_editDpt_check_clicked();

    void on_btn_editDpt_cancel_clicked();

    void on_btn_addUser_clicked();

    void on_btn_delUser_clicked();

    void on_btn_banUser_clicked();

    void on_btn_editUser_check_clicked();

    void on_btn_editUser_cancel_clicked();

    void on_btn_userManagePage_search_clicked();

    void on_btn_userManagePage_recovery_clicked();

    void on_btn_updateContent_clicked();

    void on_btn_cancelContent_clicked();

    void on_btn_addContent_clicked();

    void on_btn_delContent_clicked();

    void on_btn_searchContents_clicked();

    void on_btn_recoveryContents_clicked();

    void on_btn_oneMonth_clicked();

    void on_btn_threeMonth_clicked();

    void on_btn_removeAll_clicked();

    void on_btn_switchPanel_clicked();

    void on_btn_panel_clicked();

    void on_btn_loginCnt_clicked();

    void on_btn_registerCnt_clicked();

    void on_btn_getCnt_clicked();

    void on_btn_actCnt_clicked();

    void on_btn_dyCnt_clicked();

    void on_btn_saveSysSettings_clicked();

    void on_lineEdit_manageContents_textChanged(const QString& arg1);

    void on_comboBox_group_currentIndexChanged(const QString &arg1);

    void on_comboBox_department_currentIndexChanged(const QString &arg1);

    void on_btn_userManagePage_recovery_2_clicked();

    void on_rBtn_woman_clicked();

    void on_rBtn_man_clicked();

    void on_rBtn_all_clicked();

    void on_rBtn_actAll_clicked();

    void on_rBtn_actFinished_clicked();

    void on_rBtn_actPending_clicked();

    void on_comboBox_group_2_currentIndexChanged(const QString &arg1);

    void on_comboBox_department_2_currentIndexChanged(const QString &arg1);

    void on_checkBox_agreePrivacy_stateChanged(int state);

    void on_checkBox_autoRun_stateChanged(int state);

    void on_checkBox_noMsgRem_stateChanged(int state);

    void on_btn_resetAutoRun_clicked();

    void on_btn_attendManagePage_recovery_clicked();

    void on_btn_attendManagePage_search_clicked();

    void on_btn_attendManage_reAttend_clicked();

    void on_btn_attendManage_cancelAttend_clicked();

    void on_btn_attendManagePage_exp_clicked();

    void on_btn_expAttend_clicked();

    void on_btn_beginAttend_clicked();

    void on_btn_endAttend_clicked();

    void on_btn_switchChart_clicked();

    void on_btn_personalSubmit_clicked();

    void on_btn_personalClear_clicked();

    void on_actMessage_triggered();

    void on_action_triggered();

    void on_actManage_triggered();

    void on_actNoticeManage_triggered();

    void on_actNotice_triggered();

    void on_btn_actPush_clicked();

    void on_btn_actClear_clicked();

    void on_statusChanged(bool status);

    void on_editPersonalInfoRes(int res);

    void on_btn_getQQAvatar_clicked();

    void on_btn_verifyInfo_clicked();

    void on_btn_delVerify_clicked();
	
    void on_btn_updateVerify_clicked();

    void on_btn_sendMsg_clicked();

    void on_btn_submitHotkey_clicked();

    void on_btn_newMsgCheacked_clicked();

    void on_btn_addMsgMem_clicked();

    void on_btn_deleteMsgMem_clicked();

    void on_btn_friendInfo_clicked();
    
    void on_btn_shareMe_clicked();

    void on_btn_checkTime_clicked();

    void on_btn_reManageApplyProcess_clicked();

    void on_btn_reManageApplyOptions_clicked();

    void on_btn_manageApplyAddOption_clicked();
    
    void on_btn_manageApplyModify_clicked();

    void on_btn_manageApplyAddApply_clicked();

    void on_btn_manageApplyPublish_clicked();

	void on_btn_manageApplyDelete_clicked();

    void on_btn_manageApplySwitch_clicked();

    void on_btn_submitApply_clicked();

	void on_btn_cancelApply_clicked();
    
    void on_btn_setApplyToken_clicked();

    void on_btn_getApplyUserInfo_clicked();

	void on_btn_submitApplyResult_argee_clicked();

    void on_btn_submitApplyResult_reject_clicked();

    void on_btn_authApplyToken_clicked();

    void on_btn_smtpSave_clicked();

    void on_lineEdit_msgPushTime_textChanged(const QString& arg);

    void on_lineEdit_msgPushMaxCnt_textChanged(const QString& arg);

    void loadActMemAccountInfo(QSqlRecord rec);

signals:
    void startDbWork();

    void startBaseInfoWork();

    void startSetAuth(const QString& uid);

    void editPersonalInfo(const QString& oldPwd, const QString& tel, const QString& mail, const QString& avatar, const QString& pwd);
    
    void attendWorking();

    void attendHomeChartWorking();

    void attendPageModelSubmitAll(int type);

    void userManageWorking();

    void userManageModelSubmitAll();

    void userManageGetAvatar();

    void userManageSetCombox();

    void attendManageWorking();

    void attendManageGetAvatar();

    void attendManageModelSubmitAll(int type);

    void groupManageWorking();

    void groupManageModelSubmitAll(int type);

    void actHomeWorking();

    void activityManageWorking();

    void activityManageModelSubmitAll();

    void applyActivity(const QString aid, const QString& uid);

    void cancelActivity(const QString aid, const QString& uid);

    void approveActivity(const QString actm_id);

    void approveAllActivity(const QString act_id);

    void rejectActivity(const QString actm_id);

    void delActivityMem(const QString actm_id);

    void delActivity(const QString actm_id);

    void queryAccount(const QString& uid);

    void fixUser(int type, const QString& removedId);

    void bindQQAvatar(QString qqMail);

    void updateActStatus();

    void updateScore(float score);

    void posterWorking();

    void posterSubmitAll();

    void beginUpdate();

    void attendDataOperate(int type);

    void get_statistics();

    void poster_statistics();

    void loadStatisticsPanel();

    void getVerify(const QString& uid);

	void updateVerify(int type, int verifyTag, const QString& info);

    void loadSystemSettings();

    void saveSystemSettings();

    void loadMsgMemList(QString uid);

    void sendMessage(QByteArray array);

    void startPushMsg(QString me, QString member, int limit);

    void delFriend(const QString& me, const QString& member);

	void loadManagePageApplyItems(const QString& uid);
    
    void loadUserPageApplyItems(const QString& uid);

    void loadApplyFormList(const QString& uid);
    
	void addOrModifyApplyItem(int type, QByteArray array);  //type==0添加，type==1修改

    void deleteOrSwitchApplyItem(int type, const QString& id);  //0删除或 1开放/暂停 申请项

	void submitOrCancelApply(int type, const QString& apply_id, QByteArray array = QByteArray());  //0撤销 1提交

    void agreeOrRejectApply(const QString& apply_id, const QString& auditor, const QString& result, const QString& text);

    void getApplyToken(const QString& id);

    void authApplyToken(const QString& token);

    void saveSmtpSettings(const QString& add, const QString& user, const QString& password);
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H