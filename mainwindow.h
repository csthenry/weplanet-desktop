/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QPixmap>
#include <QDateTime>
#include <QLabel>
#include "service.h"
#include "formlogin.h"
#include "querymodel.h"
#include "comboboxdelegate.h"
#include "excelexport.h"
#include "readOnlyDelegate.h"
#include "checkupdate.h"
#include "sqlwork.h"
#include "baseinfowork.h"

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    bool dbStatus = true;

    QThread *dbThread, *sqlThread;

    QString uid, removedGroupId, removedDptId;

    QDateTime curDateTime;

    QVector<QAction*> actionList;

    QSqlDatabase db;

    QStringList comboxList;

    QLabel *connectStatusLable, *statusIcon;

    QSqlTableModel *groupModel, *departmentModel, *activityModel;  //数据模型

    QSqlRelationalTableModel *userManageModel, *attendManageModel, *attendPageModel;

    QItemSelectionModel *groupPageSelection_group, *groupPageSelection_department, *userManagePageSelection, *activitySelection; //选择模型

    queryModel *relTableModel, *relTableModel_attend;

    QDataWidgetMapper *userManagePage_dataMapper; //数据映射

    QPixmap *statusOKIcon, *statusErrorIcon, *verifyIcon;

    formLogin *formLoginWindow;

    readOnlyDelegate *readOnlyDelegate;

    ComboBoxDelegate comboxDelegateAuthority, comboxDelegateGender, comboxDelegateUserGroup, comboxDelegateUserDpt, comboxDelegateUserVerify;   //自定义数据代理

    void setHomePageBaseInfo();

    void setUsersTypeCombox(QComboBox* group, QComboBox* department);

    void setUsersFilter_group(QComboBox* group, QComboBox* department);

    void setUsersFilter_dpt(QComboBox* group, QComboBox* department);

    baseInfoWork *setBaseInfoWork;

    SqlWork *sqlWork;

public:
    MainWindow(QWidget *parent = nullptr, QDialog *formLoginWindow = nullptr);
    ~MainWindow();


private slots:
    void receiveData(QSqlDatabase db, QString uid); //接收登录窗口信号

    void on_actExit_triggered();

    void on_actHome_triggered();

    void on_actMyInfo_triggered();

    void on_actAttend_triggered();

    void on_actApply_triggered();

    void on_actUserManager_triggered();

    void on_actAttendManager_triggered();

    void on_actApplyList_triggered();

    void on_actApplyItems_triggered();

    void on_actGroup_triggered();

    void on_actMore_triggered();

    void on_groupPageDptcurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_groupPageGroupcurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_userManagePagecurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_userManagePagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_attendManagePageUserscurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_activityPagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_btn_addGroup_clicked();

    void on_btn_editGroup_check_clicked();

    void on_btn_editGroup_cancel_clicked();

    void on_btn_addDpt_clicked();

    void on_btn_delDpt_clicked();

    void on_btn_delGroup_clicked();

    void on_btn_editDpt_check_clicked();

    void on_btn_editDpt_cancel_clicked();

    void on_btn_addUser_clicked();

    void on_btn_delUser_clicked();

    void on_btn_editUser_check_clicked();

    void on_btn_editUser_cancel_clicked();

    void on_btn_userManagePage_search_clicked();

    void on_btn_userManagePage_recovery_clicked();

    void on_comboBox_group_currentIndexChanged(const QString &arg1);

    void on_comboBox_department_currentIndexChanged(const QString &arg1);

    void on_btn_userManagePage_recovery_2_clicked();

    void on_rBtn_woman_clicked();

    void on_rBtn_man_clicked();

    void on_rBtn_all_clicked();

    void on_comboBox_group_2_currentIndexChanged(const QString &arg1);

    void on_comboBox_department_2_currentIndexChanged(const QString &arg1);

    void on_btn_attendManagePage_recovery_clicked();

    void on_btn_attendManagePage_search_clicked();

    void on_btn_attendManage_reAttend_clicked();

    void on_btn_attendManage_cancelAttend_clicked();

    void on_btn_attendManagePage_exp_clicked();

    void on_btn_expAttend_clicked();

    void on_btn_beginAttend_clicked();

    void on_btn_endAttend_clicked();

    void on_PieSliceHighlight(bool show);

    void on_btn_personalSubmit_clicked();

    void on_btn_personalClear_clicked();

    void on_actMessage_triggered();

    void on_action_triggered();

    void on_actManage_triggered();

    void on_btn_actPush_clicked();

    void on_btn_actClear_clicked();

    void on_statusChanged(bool status);

    void on_actAttendManagerFinished(QSqlRelationalTableModel *curModel);

signals:
    void startDbWork();

    void startBaseInfoWork();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
