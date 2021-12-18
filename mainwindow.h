#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QPixmap>
#include <QLabel>
#include "service.h"
#include "formlogin.h"
#include "querymodel.h"
#include "comboboxdelegate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QString uid;

    QSqlDatabase db;

    QStringList comboxList;

    QLabel *connectStatusLable, *statusIcon;

    QSqlTableModel *groupModel, *departmentModel;  //数据模型

    QSqlRelationalTableModel *userManageModel;

    QItemSelectionModel *groupPageSelection_group, *groupPageSelection_department, *userManagePageSelection; //选择模型

    QDataWidgetMapper *userManagePage_dataMapper; //数据映射

    QPixmap *statusOKIcon, *statusErrorIcon, *verifyIcon;

    formLogin *formLoginWindow;

    ComboBoxDelegate comboxDelegateAuthority, comboxDelegateGender, comboxDelegateUserGroup, comboxDelegateUserDpt, comboxDelegateUserVerify;   //自定义数据代理

    void setHomePageBaseInfo();

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

    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_radioButton_3_clicked();

    void on_btn_userManagePage_search_clicked();

    void on_btn_userManagePage_recovery_clicked();

    void on_comboBox_group_currentIndexChanged(const QString &arg1);

    void on_comboBox_department_currentIndexChanged(const QString &arg1);

    void on_btn_userManagePage_recovery_2_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
