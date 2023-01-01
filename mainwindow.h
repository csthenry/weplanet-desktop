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
#include <QPixmap>
#include <QDateTime>
#include <QLabel>
#include <QMovie>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QWebChannel>
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

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QFont HarmonyOS_Font;

    PreviewPage* notice_page;

    QWebChannel* c_channel, *m_channel;

    Document m_content, c_content;

    QTimer *refTimer, *msgPushTimer;

    QMovie *loadingMovie, *avatarLoadMovie;
    
    QMutex mutex;

    bool dbStatus = true;

    bool isPushing = false; //消息推送中

    int curMsgStackCnt = 0; //当前消息栈数据量

    int msgStackMax = 30; //聊天记录最大数量

    QThread *dbThread, *sqlThread, *sqlThread_MSG, *sqlThread_MSGPUSHER, *sqlThread_SECOND;

    QString uid, removedGroupId, removedDptId, sendToUid = "-1";

    QSystemTrayIcon* trayIcon;

    QMenu *trayIconMenu;

    QAction* mShowMainAction, *mExitAppAction;

    QDateTime curDateTime;

    QVector<QAction*> actionList;

    QSqlDatabase db;

    QStringList comboxList;

    QLabel *connectStatusLable, *statusIcon;

    QSqlTableModel *groupModel, *departmentModel, *activityModel, *activityMemModel, *noticeModel, *noticeManageModel;  //数据模型

    QSqlRelationalTableModel *userManageModel, *attendManageModel, *attendPageModel;

    QDataWidgetMapper* actEditMapper, *noticeEditMapper;

    QItemSelectionModel *groupPageSelection_group, *groupPageSelection_department, *userManagePageSelection, *activitySelection, *activityMemSelection, *myActListSelection, *myActSelection, *noticeManageSelection, *noticeSelection; //选择模型

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

    int msgListTipsType = -1;
	
	int panel_series_count = 14, panel_option = -1;

    int msgPushTime = 5;

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

    void setUsersFilter_group(QComboBox* group, QComboBox* department);

    void setUsersFilter_dpt(QComboBox* group, QComboBox* department) const;

    void reloadModelBefore();   //用于relationModel跨线程刷新，一般model似乎线程安全！

    void resetUID();    //用于切换用户后重新设置Work对象中的uid

    void initToolbar(QSqlRecord rec);

    void createActions();

    void closeEvent(QCloseEvent* event);    //重写void closeEvent(QCloseEvent *event);

    void updateFinished();

    void setStatisticsPanel(int option = 0, int days = -1);
	
    void setSystemSettings();

    void setMsgPage();

    void msgPusher(QStack<QByteArray> msgStack);

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

    MsgService* msgService, *msgPusherService;

    QSettings* config_ini;

public:
    MainWindow(QWidget *parent = nullptr, QDialog *formLoginWindow = nullptr);
    ~MainWindow();


private slots:

    void receiveData(QString uid); //接收登录窗口信号

    void on_actExit_triggered();

    void on_actHome_triggered();

    void on_actMyInfo_triggered();

    void on_actAttend_triggered();

    void on_actApply_triggered() const;

    void on_actUserManager_triggered();

    void on_actAttendManager_triggered();

    void on_actApplyList_triggered() const;

    void on_actApplyItems_triggered() const;

    void on_actGroup_triggered();

    void on_actMore_triggered() const;

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

    void on_PieSliceHighlight(bool show);

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

    void on_btn_newMsgCheacked_clicked();

    void on_btn_addMsgMem_clicked();

    void on_btn_deleteMsgMem_clicked();

    void on_lineEdit_msgPushTime_textChanged(const QString& arg);

    void on_lineEdit_msgPushMaxCnt_textChanged(const QString& arg);

    void loadActMemAccountInfo(QSqlRecord rec);

signals:
    void startDbWork();

    void startBaseInfoWork();

    void startSetAuth(const QString& uid);

    void editPersonalInfo(const QString& oldPwd, const QString& tel, const QString& mail, const QString& avatar, const QString& pwd);
    void attendWorking();

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

    void beginUpdate(QLabel* label, QWidget* parent);

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

    void startPushMsg(QString uid, int limit);

    void delFriend(const QString& me, const QString& member);
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
