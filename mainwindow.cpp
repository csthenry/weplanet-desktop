/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, QDialog *formLoginWindow)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    statusIcon = new QLabel(this);     //用于显示状态图标的label
    statusIcon->setMaximumSize(25, 25);
    statusIcon->setPixmap(QPixmap(":/images/color_icon/color-setting_2.svg"));

    statusIcon->setScaledContents(true);    //图片自适应大小
    ui->avatar->setScaledContents(true);
    ui->userManagePage_avatar->setScaledContents(true);
    ui->label_verifyIcon->setScaledContents(true);
    ui->attendManagePage_avatar->setScaledContents(true);
    ui->attendPage_avatar->setScaledContents(true);
    ui->info_avatar->setScaledContents(true);

    //用户权限设置（共8个）
    actionList.append(ui->actMessage);
    actionList.append(ui->actUserManager);
    actionList.append(ui->actAttendManager);
    actionList.append(ui->actManage);
    actionList.append(ui->actApplyList);
    actionList.append(ui->actApplyItems);
    actionList.append(ui->actGroup);
    actionList.append(ui->actNoticeManage);

    connectStatusLable = new QLabel("数据库服务状态: 正在连接...");
    connectStatusLable->setMinimumWidth(1100);

    userAvatar = new QPixmap(":/images/color_icon/user.svg");
    statusOKIcon = new QPixmap(":/images/color_icon/color-approve.svg"), statusErrorIcon = new QPixmap(":/images/color_icon/color-delete.svg");

    verifyIcon = new QPixmap(":/images/color_icon/verify_2.svg");

    ui->statusbar->addWidget(statusIcon);   //将状态组件添加至statusBar
    ui->statusbar->addWidget(connectStatusLable);
    ui->stackedWidget->setCurrentIndex(13);  //转到加载首页
    connect(formLoginWindow, SIGNAL(sendData(QString)), this, SLOT(receiveData(QString)));    //接收登录窗口的信号
    readOnlyDelegate = new class readOnlyDelegate(this);    //用于tableView只读

    //设置ItemDelegate(用户管理页性别栏)
    comboxList.clear();
    comboxList << "男" << "女";
    comboxDelegateGender.setItems(comboxList, false);
    ui->tableView_userManage->setItemDelegateForColumn(3, &comboxDelegateGender);
    ui->tableView_userManage->setItemDelegate(new QSqlRelationalDelegate(ui->tableView_userManage));

    //加载动画
    loadingMovie = new QMovie(":/images/color_icon/loading.gif");
    ui->label_loading->setMovie(loadingMovie);
    loadingMovie->start();

    //心跳query
    refTimer = new QTimer(this);
    connect(refTimer, &QTimer::timeout, this, [=]()
        {
            on_actRefresh_triggered();
        });

    //托盘事件
    trayIconMenu = new QMenu(this);
    createActions();
    trayIconMenu->addAction(mShowMainAction);
    trayIconMenu->addSeparator();    //分割线
    trayIconMenu->addAction(mExitAppAction);
    trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_SystemTrayIconClicked(QSystemTrayIcon::ActivationReason)));
    QIcon icon(":/images/logo/MagicLightAssistant.png");
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("MagicLight Assistant - 运行中");
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    //多线程相关
    sqlWork = new SqlWork("mainDB");  //sql异步连接
    setBaseInfoWork = new baseInfoWork();
    attendWork = new AttendWork();
    userManageWork = new UserManageWork();
    attendManageWork = new AttendManageWork();
    groupManageWork = new GroupManageWork();
    activityManageWork = new ActivityManageWork();
    posterWork = new PosterWork();

    sqlThread = new QThread(), dbThread = new QThread();
    sqlWork->moveToThread(dbThread);
    setBaseInfoWork->moveToThread(sqlThread);
    attendWork->moveToThread(sqlThread);
    userManageWork->moveToThread(sqlThread);
    attendManageWork->moveToThread(sqlThread);
    groupManageWork->moveToThread(sqlThread);
    activityManageWork->moveToThread(sqlThread);
    posterWork->moveToThread(sqlThread);
    
    //检查更新
    updateSoftWare.moveToThread(sqlThread);

    connect(this, &MainWindow::beginUpdate, &updateSoftWare, &checkUpdate::parse_UpdateJson);
    connect(&updateSoftWare, &checkUpdate::finished, this, &MainWindow::updateFinished);
    emit beginUpdate(ui->notice, this);

    //开启数据库连接线程
    dbThread->start();
    sqlWork->beginThread();
    connect(this, &MainWindow::startDbWork, sqlWork, &SqlWork::working);
    emit startDbWork();

    connect(sqlWork, SIGNAL(newStatus(bool)), this, SLOT(on_statusChanged(bool)));    //数据库心跳验证 5s

    //注册一些信号槽所需
    qRegisterMetaType<QSqlRecord>("QSqlRecord"); 
	qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    //sqlWork firstFinished信号槽
    connect(sqlWork, &SqlWork::firstFinished, this, [=](){
        sqlWork->stopThread();

        //构造model
        attendPageModel = new QSqlRelationalTableModel(this, attendWork->getDB());
        userManageModel = new QSqlRelationalTableModel(nullptr, userManageWork->getDB());
        attendManageModel = new QSqlRelationalTableModel(this, attendManageWork->getDB());
        groupModel = new QSqlTableModel(this, groupManageWork->getDB());
        departmentModel = new QSqlTableModel(this, groupManageWork->getDB());
        activityModel = new QSqlTableModel(this, activityManageWork->getDB());
        activityMemModel = new QSqlTableModel(this, activityManageWork->getDB());
        noticeModel = new QSqlTableModel(this, posterWork->getDB());
        noticeManageModel = new QSqlTableModel(this, posterWork->getDB());

        userManagePageSelection = new QItemSelectionModel(userManageModel);
        groupPageSelection_group = new QItemSelectionModel(groupModel);
        groupPageSelection_department = new QItemSelectionModel(departmentModel);
        activitySelection = new QItemSelectionModel(activityModel);
        activityMemSelection = new QItemSelectionModel(activityMemModel);
        myActListSelection = new QItemSelectionModel(activityModel);
        myActSelection = new QItemSelectionModel(activityMemModel);
        noticeManagerSelection = new QItemSelectionModel(noticeManageModel);

        //构造mapper
        actEditMapper = new QDataWidgetMapper(this);
        noticeEditMapper = new QDataWidgetMapper(this);

        //初始化work
        setBaseInfoWork->setUid(uid);
        attendWork->setModel(attendPageModel);
        attendWork->setUid(uid);

        userManageWork->setModel(userManageModel);
        userManageWork->setCombox(ui->comboBox_group, ui->comboBox_department);

        attendManageWork->setUserModel(userManageModel);    //直接沿用用户管理界面的model
        attendManageWork->setAttendModel(attendManageModel);
        attendManageWork->setCombox(ui->comboBox_group_2, ui->comboBox_department_2);

        groupManageWork->setGroupModel(groupModel);
        groupManageWork->setDepartmentModel(departmentModel);

        activityManageWork->setModel(activityModel);
        activityManageWork->setMemberModel(activityMemModel);

        posterWork->setManageModel(noticeManageModel);
        posterWork->setModel(noticeModel);


        emit startSetAuth(uid);
        emit startBaseInfoWork();   //等待数据库第一次连接成功后再调用
        emit actHomeWorking();
        refTimer->start(5*60*1000);  //开启心跳query定时器（5分钟心跳）
    }, Qt::UniqueConnection);

    //个人基本信息信号槽
    connect(setBaseInfoWork, &baseInfoWork::baseInfoFinished, this, &MainWindow::setHomePageBaseInfo);
    sqlThread->start();
    connect(this, &MainWindow::startBaseInfoWork, setBaseInfoWork, &baseInfoWork::loadBaseInfoWorking);

    //首页活动信号槽
    connect(this, &MainWindow::actHomeWorking, activityManageWork, &ActivityManageWork::homeWorking);
     connect(activityManageWork, &ActivityManageWork::actHomeWorkFinished, this, [=]()
         {
             ui->tableView_activityHome->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
             ui->tableView_activityHome->setModel(activityModel);
             ui->tableView_activityHome->hideColumn(3);
             ui->tableView_activityHome->setSelectionBehavior(QAbstractItemView::SelectRows);
             ui->tableView_activityHome->setSelectionMode(QAbstractItemView::SingleSelection);
			 ui->tableView_activityHome->setAlternatingRowColors(true);
             ui->tableView_activityHome->setEditTriggers(QAbstractItemView::NoEditTriggers);
             ui->stackedWidget->setCurrentIndex(0);
         });
    //账号权限验证信号槽
    connect(this, SIGNAL(startSetAuth(const QString&)), setBaseInfoWork, SLOT(setAuthority(const QString&)));
    connect(setBaseInfoWork, &baseInfoWork::authorityRes, this, [=](QSqlRecord res){
        if(res.value(0).toString().isEmpty())
        {
            QMessageBox::warning(this, "警告", "用户权限校验失败，请关闭程序后重试。", QMessageBox::Ok);
            this->setEnabled(false);
        }
        else
        {
            initToolbar(res);
            this->setEnabled(true);
        }
    });

    //个人信息编辑信号槽
    connect(this, SIGNAL(editPersonalInfo(const QString&, const QString&, const QString&, const QString&, const QString&)), setBaseInfoWork, SLOT(editPersonalInfo(const QString&, const QString&, const QString&, const QString&, const QString&)));
    connect(setBaseInfoWork, SIGNAL(editPersonalInfoRes(int)), this, SLOT(on_editPersonalInfoRes(int)));
    connect(this, &MainWindow::bindQQAvatar, setBaseInfoWork, &baseInfoWork::bindQQAvatar);
    connect(setBaseInfoWork, &baseInfoWork::bindQQAvatarFinished, this, [=](int tag)
    {
    	disconnect(loadingMovie, &QMovie::frameChanged, this, 0);
    	ui->btn_getQQAvatar->setIcon(QIcon(QPixmap(":/images/color_icon/user.svg")));
    	if (tag == 1)
    	{
    		emit startBaseInfoWork();      //刷新个人信息
    		QMessageBox::information(this, "消息", "QQ头像绑定成功，你的头像将会随QQ头像更新。", QMessageBox::Ok);
    	}
        else if(tag == 0)
            QMessageBox::warning(this, "错误", "我们很想获取你的QQ头像，但不可思议的是，你的邮箱竟然不是QQ邮箱...请更换QQ邮箱后再试吧~", QMessageBox::Ok);
        else
            QMessageBox::warning(this, "错误", "未知错误，请检查网络情况或联系管理员。", QMessageBox::Ok);
    });

    //个人考勤页面信号槽
    connect(this, &MainWindow::attendWorking, attendWork, &AttendWork::working);
    connect(attendWork, &AttendWork::attendWorkFinished, this, &MainWindow::setAttendPage);
    connect(this, SIGNAL(attendPageModelSubmitAll(int)), attendWork, SLOT(submitAll(int)));
    connect(attendWork, &AttendWork::attendDone, this, [=](bool res){
        if(res)
        {
            QMessageBox::information(this, "消息", "签到成功，签到时间:\n" + curDateTime.toString("yyyy-MM-dd hh:mm:ss") + " 祝你今天元气满满~", QMessageBox::Ok);
            on_actAttend_triggered();  //刷新信息
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + attendPageModel->lastError().text(), QMessageBox::Ok);
    }, Qt::UniqueConnection);
    connect(attendWork, &AttendWork::attendOutDone, this, [=](bool res){
        if(res)
        {
            QMessageBox::information(this, "消息", "签退成功，签退时间：" + curDateTime.toString("yyyy-MM-dd hh:mm:ss") + " 累了一天了，休息一下吧~", QMessageBox::Ok);
            on_actAttend_triggered();  //刷新信息
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + attendPageModel->lastError().text(), QMessageBox::Ok);
    }, Qt::UniqueConnection);

    //用户管理信号槽
    connect(this, &MainWindow::userManageWorking, userManageWork, &UserManageWork::working);
    connect(this, &MainWindow::userManageGetAvatar, userManageWork, &UserManageWork::loadAvatar);
    connect(this, &MainWindow::userManageModelSubmitAll, userManageWork, &UserManageWork::submitAll);
    connect(userManageWork, &UserManageWork::userManageWorkFinished, this, &MainWindow::setUserManagePage);
    connect(userManageWork, &UserManageWork::avatarFinished, this, [=](QPixmap avatar){
        if(avatar.isNull())
            ui->userManagePage_avatar->setPixmap(*userAvatar);
        else
            ui->userManagePage_avatar->setPixmap(service::setAvatarStyle(avatar));
    }, Qt::UniqueConnection);
    connect(userManageWork, &UserManageWork::submitAllFinished, this, [=](bool res){
        if(!res)
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + userManageModel->lastError().text(),
                                     QMessageBox::Ok);
        else{
            ui->btn_editUser_check->setEnabled(false);
            ui->btn_editUser_cancel->setEnabled(false);
        }
    }, Qt::UniqueConnection);

    //考勤管理信号槽
    connect(this, &MainWindow::attendManageWorking, attendManageWork, &AttendManageWork::working);
    connect(this, &MainWindow::attendManageGetAvatar, attendManageWork, &AttendManageWork::loadAvatar);
    connect(this, &MainWindow::attendManageModelSubmitAll, attendManageWork, &AttendManageWork::submitAll);
    connect(attendManageWork, &AttendManageWork::attendManageWorkFinished, this, &MainWindow::setAttendManagePage);
    connect(attendManageWork, &AttendManageWork::avatarFinished, this, [=](QPixmap avatar){
        if(avatar.isNull())
            ui->attendManagePage_avatar->setPixmap(*userAvatar);
        else
            ui->attendManagePage_avatar->setPixmap(service::setAvatarStyle(avatar));
    }, Qt::UniqueConnection);
    connect(attendManageWork, &AttendManageWork::submitAddFinished, this, [=](bool res){
        if(res)
        {
            QMessageBox::information(this, "消息", "补签成功，补签时间:\n" + curDateTime.toString("yyyy-MM-dd hh:mm:ss"),
                                     QMessageBox::Ok);
            ui->label_attendManagePage_status->setText("已签到");
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + attendManageModel->lastError().text(),
                                     QMessageBox::Ok);
    });
    connect(attendManageWork, &AttendManageWork::submitDelFinished, this, [=](bool res){
        if(res)
        {
            QMessageBox::information(this, "消息", "当前用户已被成功退签，当日签到数据已经删除。", QMessageBox::Ok);
            ui->label_attendManagePage_status->setText("未签到");
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + attendManageModel->lastError().text(),
                                     QMessageBox::Ok);
    });
    //活动管理信号槽
    connect(this, &MainWindow::activityManageWorking, activityManageWork, &ActivityManageWork::working);
    connect(this, &MainWindow::activityManageModelSubmitAll, activityManageWork, &ActivityManageWork::submitAll);
    connect(this, &MainWindow::approveActivity, activityManageWork, &ActivityManageWork::m_approve);
    connect(this, &MainWindow::rejectActivity, activityManageWork, &ActivityManageWork::m_reject);
    connect(this, &MainWindow::delActivityMem, activityManageWork, &ActivityManageWork::m_delete);
    connect(this, &MainWindow::delActivity, activityManageWork, &ActivityManageWork::delActivity);
    connect(this, &MainWindow::queryAccount, userManageWork, &UserManageWork::queryAccount);
    connect(userManageWork, &UserManageWork::queryAccountFinished, this, &MainWindow::loadActMemAccountInfo);
    connect(userManageWork, &UserManageWork::avatarFinished, this, [=](QPixmap avatar) {
        if (ui->stackedWidget->currentIndex() == 8)
        {
            if (avatar.isNull())
                ui->activityManage_avatar->setPixmap(*userAvatar);
            else
                ui->activityManage_avatar->setPixmap(service::setAvatarStyle(avatar));
            ui->userManagePage_avatar->setPixmap(*userAvatar);  //避免影响用户管理页面的头像，因为avatarFinished绑定了两个槽
        }
        }, Qt::UniqueConnection);
    connect(activityManageWork, &ActivityManageWork::activityManageWorkFinished, this, [=](int type)
    {
	    if(type == 1)
            setActivityPage();
        else
            setActivityManagePage();
    }, Qt::UniqueConnection);
    connect(activityManageWork, &ActivityManageWork::submitAllFinished, this, [=](bool res) {
        if (!res)
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + activityModel->lastError().text(),
                QMessageBox::Ok);
        else
        {
            QMessageBox::information(this, "消息", "活动【" + ui->lineEdit_actName->text() + "】发布成功，请注意审核活动报名成员。"
                , QMessageBox::Ok);
            ui->lineEdit_actName->clear();
            ui->textEdit_activity->clear();
            ui->spinBox_actScore->setValue(0);
            on_actManage_triggered();
        }
     });
    connect(activityManageWork, &ActivityManageWork::manageOperateFinished, this, [=](QString res)
		{
            if (res.isEmpty())
                QMessageBox::information(this, "消息", "[录取/拒绝/删除] 操作完成，页面即将刷新。"
                    , QMessageBox::Ok);
            else
                QMessageBox::information(this, "消息", "操作失败，请检查你的网络连接。\n错误信息：" + res
                    , QMessageBox::Ok);
            on_actManage_triggered();
		});

    //活动页信号槽
    connect(this, &MainWindow::applyActivity, activityManageWork, &ActivityManageWork::apply);
    connect(this, &MainWindow::cancelActivity, activityManageWork, &ActivityManageWork::cancel);
    connect(this, &MainWindow::updateActStatus, activityManageWork, &ActivityManageWork::updateActStatus);
    connect(this, &MainWindow::updateScore, setBaseInfoWork, &baseInfoWork::updateScore);
    connect(activityManageWork, &ActivityManageWork::operateFinished, this, [=](QString res)
        {
            if (res.isEmpty())
                QMessageBox::information(this, "消息", "操作完成，请点击[我的活动]查看已报名活动。"
                    , QMessageBox::Ok);
            else
                QMessageBox::information(this, "消息", "操作失败，请检查你的网络连接。\n错误信息：" + res
                    , QMessageBox::Ok);
            on_action_triggered();  //刷新页面
        });

    //通知动态信号槽
    connect(this, &MainWindow::posterWorking, posterWork, &PosterWork::working);
    connect(posterWork, &PosterWork::contentsManageWorkFinished, this, &MainWindow::setNoticeManagePage);

    //组织架构管理信号槽
    connect(this, &MainWindow::groupManageWorking, groupManageWork, &GroupManageWork::working);
    connect(this, &MainWindow::groupManageModelSubmitAll, groupManageWork, &GroupManageWork::submitAll);
    connect(groupManageWork, &GroupManageWork::groupManageWorkFinished, this, &MainWindow::setGroupManagePage);
    connect(this, &MainWindow::fixUser, groupManageWork, &GroupManageWork::fixUser);
    connect(groupManageWork, &GroupManageWork::submitFinished_0, this, [=](bool res){
        if (!res)
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + departmentModel->lastError().text(),
                                     QMessageBox::Ok);
        else
        {
            ui->btn_editDpt_check->setEnabled(departmentModel->isDirty());
            ui->btn_editDpt_cancel->setEnabled(departmentModel->isDirty());
            if(!removedDptId.isEmpty())
            {
                emit fixUser(0, removedDptId);
                removedDptId.clear();
            }
        }
    });

    connect(groupManageWork, &GroupManageWork::submitFinished_1, this, [=](bool res){
        if (!res)
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + groupModel->lastError().text(),
                                     QMessageBox::Ok);
        else
        {
            ui->btn_editGroup_check->setEnabled(false);
            ui->btn_editGroup_cancel->setEnabled(false);
            if(!removedGroupId.isEmpty())
            {
                emit fixUser(1, removedGroupId);
                removedGroupId.clear();
            }
        }
    });
}
MainWindow::~MainWindow()
{
    //在此处等待所有线程停止
    if(sqlThread->isRunning())
    {
        sqlThread->quit();
        sqlThread->wait();
    }
    if(dbThread->isRunning())
    {
        sqlWork->stopThread();
        sqlWork->quit();
        dbThread->quit();
        dbThread->wait();
    }
    refTimer->stop();
    loadingMovie->stop();
    delete loadingMovie;

    //析构所有工作对象和线程
    delete ui;

    delete sqlWork;
    delete setBaseInfoWork;
    delete userManageWork;
    delete attendManageWork;
    delete groupManageWork;
    delete activityManageWork;

    delete sqlThread;
    delete dbThread;

    delete readOnlyDelegate;
}

void MainWindow::receiveData(QString uid)
{
    this->setEnabled(false);
    this->uid = uid;
    ui->label_home_uid->setText(uid);
    ui->label_info_uid->setText(uid);
    curDateTime = QDateTime::currentDateTime();
    ui->dateTimeEdit_actJoin->setDateTime(curDateTime);
    ui->dateTimeEdit_actBegin->setDateTime(curDateTime);
    ui->dateTimeEdit_actEnd->setDateTime(curDateTime);
}

void MainWindow::updateFinished()
{
    ui->groupBox_33->setTitle("版本公告（软件版本：Ver " + updateSoftWare.getCurVersion() + "）");
    ui->label_homeVer->setText("Ver " + updateSoftWare.getCurVersion());
    if (updateSoftWare.getLatestVersion().isEmpty())
        ui->label_LatestVersion->setText("--");
    else
        ui->label_LatestVersion->setText(updateSoftWare.getLatestVersion());
}

void MainWindow::setHomePageBaseInfo()
{   
    ui->label_home_name->setText(setBaseInfoWork->getName());
    ui->label_info_name->setText(setBaseInfoWork->getName());
    ui->label_home_gender->setText(setBaseInfoWork->getGender());
    ui->label_info_gender->setText(setBaseInfoWork->getGender());
    ui->label_home_tel->setText(setBaseInfoWork->getTel());
    ui->label_info_tel->setText(setBaseInfoWork->getTel());
    ui->label_home_mail->setText(setBaseInfoWork->getMail());
    ui->label_info_mail->setText(setBaseInfoWork->getMail());
    ui->label_home_group->setText(setBaseInfoWork->getGroup());
    ui->label_info_group->setText(setBaseInfoWork->getGroup());
    ui->label_home_department->setText(setBaseInfoWork->getDepartment());
    ui->label_info_department->setText(setBaseInfoWork->getDepartment());
    ui->label_home_score->setText(setBaseInfoWork->getScore());

    if(ui->label_home_gender->text().isEmpty())
    {
        ui->label_home_gender->setText("--");   //将可能为空的数据设置值
        ui->label_info_gender->setText("--");
    }
    if(ui->label_home_tel->text().isEmpty())
    {
        ui->label_home_tel->setText("--");
        ui->label_info_tel->setText("--");
    }
    if(ui->label_home_mail->text().isEmpty())
    {
        ui->label_home_mail->setText("--");
        ui->label_info_mail->setText("--");
    }
    if (ui->label_home_score->text().isEmpty())
        ui->label_home_score->setText("--");

    if(setBaseInfoWork->getAvatar().isNull())
        ui->avatar->setPixmap(*userAvatar);
    else
        ui->avatar->setPixmap(service::setAvatarStyle(setBaseInfoWork->getAvatar()));
    ui->info_avatar->setPixmap(*ui->avatar->pixmap());

    //考勤页面用户信息
    ui->label_attendPage_uid->setText(uid);
    ui->label_attendPage_name->setText(setBaseInfoWork->getName());
    ui->label_attendPage_group->setText(setBaseInfoWork->getGroup());
    ui->label_attendPage_dpt->setText(setBaseInfoWork->getDepartment());
    ui->attendPage_avatar->setPixmap(*ui->avatar->pixmap());

    //首页考勤信息初始化
    curDateTime = QDateTime::currentDateTime();
    ui->label_homePage_attendDate->setText(curDateTime.date().toString("yyyy年MM月dd日"));

    if(setBaseInfoWork->getAttendToday())
    {
        ui->label_homePage_attendStatus->setText("已签到");
        ui->label_homePage_beginTime->setText(setBaseInfoWork->getBeginTime());
        if(setBaseInfoWork->getEndTime().isNull())
            ui->label_homePage_endTime->setText("--");
        else
            ui->label_homePage_endTime->setText(setBaseInfoWork->getEndTime());
    }
    else
    {
        ui->label_homePage_attendStatus->setText("未签到");
        ui->label_homePage_beginTime->setText("--");
        ui->label_homePage_endTime->setText("--");
    }
    sqlWork->beginThread();
}

void MainWindow::setUsersFilter_group(QComboBox *group, QComboBox *department)
{

    QString sqlWhere = "group_name='" + group->currentText() + "'";
    if(department->currentIndex() != 0)
        sqlWhere += " AND dpt_name='" + department->currentText() + "'";
    if(group->currentIndex() == 0)
    {
        if(department->currentIndex() != 0)
            sqlWhere = "dpt_name='" + department->currentText() + "'";
        else
            sqlWhere.clear();
    }
    userManageModel->setFilter(sqlWhere);
}

void MainWindow::setUsersFilter_dpt(QComboBox *group, QComboBox *department) const
{
    QString sqlWhere = "dpt_name='" + department->currentText() + "'";
    if(group->currentIndex() != 0)
        sqlWhere += " AND group_name='" + group->currentText() + "'";
    if(department->currentIndex() == 0)
    {
        if(group->currentIndex() != 0)
            sqlWhere = "group_name='" + group->currentText() + "'";
        else
            sqlWhere.clear();
    }
    userManageModel->setFilter(sqlWhere);
}

void MainWindow::reloadModelBefore()
{
    //此函数用于在刷新model前，对relationModel clear，避免发生跨线程冲突
    attendPageModel->clear();
    userManageModel->clear();
    attendManageModel->clear();
}

void MainWindow::resetUID()
{
    attendWork->setUid(uid);
    setBaseInfoWork->setUid(uid);
}


void MainWindow::on_actExit_triggered()
{
    QSettings settings("bytecho", "MagicLightAssistant");
    settings.setValue("isAutoLogin", false);    //注销后自动登录失效

    // QSqlDatabase::removeDatabase("loginDB");
    // QSqlDatabase::removeDatabase("test_loginDB");
    formLoginWindow = new formLogin();
    refTimer->stop();
    trayIcon->hide();
    this->close();
    connect(formLoginWindow, SIGNAL(sendData(QString)), this, SLOT(receiveData(QString)));    //接收登录窗口的信号
    if (formLoginWindow->exec() == QDialog::Accepted)
    {
        formLoginWindow->send();    //发送信号
        resetUID();
        emit startSetAuth(uid);
        emit startBaseInfoWork();
        emit actHomeWorking();
        ui->stackedWidget->setCurrentIndex(13);  //回到首页
        refTimer->start(3 * 60 * 1000);  //开启心跳query定时器（3分钟心跳）
        delete formLoginWindow;
        this->showMinimized();
        QThread::msleep(150);
        this->showNormal();
        trayIcon->show();
        return;
    }
    delete formLoginWindow;
}

void MainWindow::on_actHome_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    if(sqlWork->getisPaused())
        sqlWork->stopThread();  //等待sqlWork暂停时再停止，避免数据库未连接
    else
        return;
    emit startBaseInfoWork();   //刷新首页数据
    emit actHomeWorking();
}

void MainWindow::on_actMyInfo_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
    QRegExp regx_pwd("[0-9A-Za-z!@#$%^&*.?]{1,16}$"), regx_num("[0-9]{1,11}$");
    QValidator* validator_pwd = new QRegExpValidator(regx_pwd), * validator_tel = new QRegExpValidator(regx_num);
    ui->lineEdit_personalTel->setValidator(validator_tel);
    ui->lineEdit_personalPwd->setValidator(validator_pwd);
    ui->lineEdit_editPwdCheck->setValidator(validator_pwd);
}

void MainWindow::on_actAttend_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    ui->tableView_attendPage->setSelectionBehavior(QAbstractItemView::SelectRows);

    reloadModelBefore();
    emit attendWorking();
}

void MainWindow::setAttendPage()
{
    curDateTime = QDateTime::currentDateTime();

    ui->tableView_attendPage->setModel(attendPageModel);
    ui->tableView_attendPage->hideColumn(0);   //隐藏考勤数据编号
    ui->tableView_attendPage->setEditTriggers(QAbstractItemView::NoEditTriggers); //不可编辑
    QSqlRecord curRec = attendWork->getRecord(0);     //取最新的一条记录
    if(curRec.value("today") == curDateTime.date().toString("yyyy-MM-dd"))
    {
        ui->label_attendPage_status->setText("已签到");
        ui->label_attendPage_beginTime->setText(curRec.value("begin_date").toString());
        ui->label_attendPage_endTime->setText(curRec.value("end_date").toString());
        if(curRec.value("isSupply") == "是")
            ui->label_attendPage_isSupply->setText("<补签>");
        else
            ui->label_attendPage_isSupply->setText("");
        if(curRec.value("end_date").toString().isEmpty())
            ui->label_attendPage_endTime->setText("--");
    }
    else
    {
        ui->label_attendPage_status->setText("未签到");
        ui->label_attendPage_isSupply->setText("");
        ui->label_attendPage_beginTime->setText("--");
        ui->label_attendPage_endTime->setText("--");
    }
    ui->stackedWidget->setCurrentIndex(4);
    ui->stackedWidget->currentWidget()->setEnabled(true);

    int *workTimeSum = attendWork->getWorkTime();
    service::buildAttendChart(ui->chartView_attend, this, ui->label->font(), workTimeSum[0], workTimeSum[1], workTimeSum[2], workTimeSum[3]);  //绘制统计图
}

void MainWindow::on_PieSliceHighlight(bool show)
{ //鼠标移入、移出时触发hovered()信号，动态设置setExploded()效果
    QPieSlice *slice;
    slice = (QPieSlice *)sender();
//    slice->setLabelVisible(show);
    slice->setExploded(show);
}
void MainWindow::on_actApply_triggered() const
{
    ui->stackedWidget->setCurrentIndex(5);
}

void MainWindow::on_actUserManager_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    ui->tableView_userManage->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_userManage->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_userManage->setItemDelegateForColumn(0, readOnlyDelegate);    //UID不可编辑
    reloadModelBefore();
    emit userManageWorking();
}

void MainWindow::setUserManagePage() const
{
    ui->tableView_userManage->setModel(userManageModel);
    ui->tableView_userManage->hideColumn(1);  //隐藏密码列
    ui->tableView_userManage->hideColumn(10);  //隐藏用户状态
	ui->tableView_userManage->setSelectionModel(userManagePageSelection);
    //当前项变化时触发currentChanged信号
    connect(userManagePageSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                this, SLOT(on_userManagePagecurrentChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    
    //当前行变化时触发currentRowChanged信号
    connect(userManagePageSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                this, SLOT(on_userManagePagecurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);

    ui->stackedWidget->setCurrentIndex(6);
    ui->stackedWidget->currentWidget()->setEnabled(true);
}

void MainWindow::on_actAttendManager_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    ui->tableView_attendUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_attendUsers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_attendInfo->setItemDelegateForColumn(1, readOnlyDelegate);     //第一列不可编辑，因为隐藏了第一列，所以列号是1不是0
    ui->tableView_attendInfo->setItemDelegateForColumn(5, readOnlyDelegate);
    ui->tableView_attendInfo->setItemDelegateForColumn(6, readOnlyDelegate);
    ui->tableView_attendUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);  //不可编辑
    ui->tableView_attendInfo->setSelectionBehavior(QAbstractItemView::SelectRows);

    reloadModelBefore();
    emit attendManageWorking();
}

void MainWindow::setAttendManagePage() const
{
    //用户列表
    ui->tableView_attendUsers->setModel(userManageModel);
    ui->tableView_attendUsers->hideColumn(1);  //隐藏密码
    ui->tableView_attendUsers->hideColumn(8);  //头像地址
    ui->tableView_attendUsers->hideColumn(9);  //学时
    ui->tableView_attendUsers->hideColumn(10); //用户状态
    
    ui->tableView_attendUsers->setSelectionModel(userManagePageSelection);
    
    //当前行变化时触发currentRowChanged信号
    connect(userManagePageSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                this, SLOT(on_attendManagePageUserscurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    
    //签到列表
    ui->tableView_attendInfo->setModel(attendManageModel);
    ui->tableView_attendInfo->hideColumn(0);     //隐藏不需要的签到编号

    ui->stackedWidget->setCurrentIndex(7);
    ui->stackedWidget->currentWidget()->setEnabled(true);
}

void MainWindow::setActivityManagePage()
{
    //活动列表
    ui->tableView_actList->setModel(activityModel);
    ui->tableView_actList->setSelectionModel(activitySelection);
    ui->tableView_actMember->setModel(activityMemModel);
    ui->tableView_actMember->setSelectionModel(activityMemSelection);

    actEditMapper->setModel(activityModel);
    actEditMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    actEditMapper->addMapping(ui->lineEdit_actName_2, 1);
    actEditMapper->addMapping(ui->textEdit_activity_2, 2);
    actEditMapper->addMapping(ui->dateTimeEdit_actJoin_2, 3);
    actEditMapper->addMapping(ui->dateTimeEdit_actBegin_2, 4);
    actEditMapper->addMapping(ui->dateTimeEdit_actEnd_2, 5);
    actEditMapper->addMapping(ui->spinBox_actScore_2, 7);
    //actEditMapper->toFirst();

    //当前行变化时触发currentChanged信号
    connect(activitySelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
        this, SLOT(on_activityManagePagecurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    connect(activityMemSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
        this, SLOT(on_activityManagePageMemcurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);

    activityMemModel->setFilter("act_id='--'");
    activityModel->setFilter("editUid=" + uid);     //仅能管理自己发布的活动
    ui->stackedWidget->setCurrentIndex(8);
    ui->stackedWidget->currentWidget()->setEnabled(true);
}

void MainWindow::setNoticeManagePage()
{
    ui->stackedWidget->setCurrentIndex(14);

    ui->tableView_mContents->setModel(noticeManageModel);
    ui->tableView_mContents->setSelectionModel(noticeManagerSelection);
    noticeEditMapper->setModel(noticeManageModel);
    noticeEditMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    ui->tableView_mContents->hideColumn(2);  //隐藏一些列
    ui->tableView_mContents->hideColumn(3);
    ui->tableView_mContents->hideColumn(4);
    ui->tableView_mContents->hideColumn(5);
    ui->tableView_mContents->hideColumn(6);
    ui->tableView_mContents->hideColumn(7);

}

void MainWindow::loadActMemAccountInfo(QSqlRecord rec)
{
    if (rec.value("uid").toString().isEmpty())
        ui->label_actMemUid->setText("--");
    else
        ui->label_actMemUid->setText(rec.value("uid").toString());

    if (rec.value("name").toString().isEmpty())
        ui->label_actMemName->setText("--");
    else
        ui->label_actMemName->setText(rec.value("name").toString());

    if (rec.value("user_group").toString().isEmpty())
        ui->label_actMemGroup->setText("--");
    else
        ui->label_actMemGroup->setText(rec.value("user_group").toString());

    if (rec.value("user_dpt").toString().isEmpty())
        ui->label_actMemDpt->setText("--");
    else
        ui->label_actMemDpt->setText(rec.value("user_dpt").toString());

    if (rec.value("telephone").toString().isEmpty())
        ui->label_actTel->setText("--");
    else
        ui->label_actTel->setText(rec.value("telephone").toString());

    if (rec.value("mail").toString().isEmpty())
        ui->label_actMail->setText("--");
    else
        ui->label_actMail->setText(rec.value("mail").toString());

    //子线程加载头像
    userManageWork->setCurAvatarUrl(rec.value("user_avatar").toString());
    emit userManageGetAvatar();
}

void MainWindow::on_action_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    ui->lineEdit_actSearch->clear();
    if(ui->comboBox_activity->currentIndex() != 0)
		ui->comboBox_activity->setCurrentIndex(0);
    activityManageWork->setType(1);
    activityManageWork->setUid(uid);
    emit activityManageWorking();
    ui->tableView_activity->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_activity->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_activity->setAlternatingRowColors(true);
    ui->tableView_myActivity->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_myActivity->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_myActivity->setAlternatingRowColors(true);
    ui->tableView_activity->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_myActivity->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::setActivityPage()
{
    ui->tableView_activity->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_activity->setModel(activityModel);
    ui->tableView_myActivity->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//填充整个view
    ui->tableView_myActivity->setModel(activityMemModel);
    ui->tableView_myActivity->hideColumn(2);

    ui->tableView_activity->setSelectionModel(myActListSelection);
    ui->tableView_myActivity->setSelectionModel(myActSelection);
    //当前行变化时触发currentRowChanged信号
    connect(myActListSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
        this, SLOT(on_activityPagecurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    connect(myActSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
        this, SLOT(on_myActivityPagecurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    activityMemModel->setFilter("actm_uid=" + uid);
    ui->comboBox_myAct->setCurrentIndex(0);
    ui->comboBox_activity->setCurrentIndex(0);

    float score = activityManageWork->getCurScore();    //getCurScore()会清空当前待添加学时
    if (score > 0)
        emit updateScore(score);    //如果待添加学时不为0，则写入用户数据库

    ui->stackedWidget->setCurrentIndex(3);
    ui->stackedWidget->currentWidget()->setEnabled(true);
}

void MainWindow::on_actManage_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);

    activityManageWork->setType(2);
    emit activityManageWorking();
    curDateTime = QDateTime::currentDateTime();
    // ui->dateTimeEdit_actBegin->setDateTime(curDateTime);
    // ui->dateTimeEdit_actEnd->setDateTime(curDateTime);
    // ui->dateTimeEdit_actJoin->setDateTime(curDateTime);

    ui->tableView_actList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_actList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_actMember->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_actMember->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tableView_actList->setItemDelegateForColumn(0, readOnlyDelegate);
    ui->tableView_actList->setItemDelegateForColumn(6, readOnlyDelegate);
    ui->tableView_actMember->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::on_actMessage_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_actNotice_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
}

void MainWindow::on_actNoticeManage_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    posterWork->setWorkType(1);
    emit posterWorking();

	//初始化Markdown解析
    PreviewPage* notice_page = new PreviewPage(this);
    ui->mContents_preview->setPage(notice_page);

    connect(ui->contents_editor, &QPlainTextEdit::textChanged,
        [this]() { m_content.setText(ui->contents_editor->toPlainText()); });

    QWebChannel* channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("content"), &m_content);
    notice_page->setWebChannel(channel);

    ui->mContents_preview->setUrl(QUrl("qrc:/images/index.html"));

    ui->tableView_mContents->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_mContents->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_mContents->setEditTriggers(QAbstractItemView::NoEditTriggers);    //禁止编辑

}

void MainWindow::on_actApplyList_triggered() const
{
    ui->stackedWidget->setCurrentIndex(9);
}

void MainWindow::on_actApplyItems_triggered() const
{
    ui->stackedWidget->setCurrentIndex(10);
}

void MainWindow::on_actGroup_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);

    emit groupManageWorking();      //开始加载model

    //tableView显示属性设置
    ui->tableView_group->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_group->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_group->setAlternatingRowColors(true);
    ui->tableView_department->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_department->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_department->setAlternatingRowColors(true);
    //ui->tableView->resizeColumnsToContents();
    //ui->tableView->horizontalHeader()->setStretchLastSection(true);

    ui->tableView_department->setItemDelegateForColumn(0, readOnlyDelegate);    //第一列不可编辑
    ui->tableView_group->setItemDelegateForColumn(0, readOnlyDelegate);

    //当前项变化时触发currentChanged信号
    connect(groupPageSelection_group, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                this, SLOT(on_groupPageGroupcurrentChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    //选择行变化时
    connect(groupPageSelection_department, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                this, SLOT(on_groupPageDptcurrentChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);

    ui->tableView_group->setRowHidden(0, true);

}

void MainWindow::setGroupManagePage()
{
    bool isEditable = false;
    ui->tableView_department->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_group->setModel(groupModel);
    ui->tableView_department->setModel(departmentModel);

    //权限设置combox
    comboxList.clear();
    comboxList << "0" << "1";
    comboxDelegateAuthority.setItems(comboxList, isEditable);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("users_manage"), &comboxDelegateAuthority);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("attend_manage"), &comboxDelegateAuthority);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("apply_manage"), &comboxDelegateAuthority);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("applyItem_manage"), &comboxDelegateAuthority);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("group_manage"), &comboxDelegateAuthority);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("activity_manage"), &comboxDelegateAuthority);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("send_message"), &comboxDelegateAuthority);
    ui->tableView_group->setItemDelegateForColumn(groupModel->fieldIndex("notice_manage"), &comboxDelegateAuthority);

    ui->tableView_group->setSelectionModel(groupPageSelection_group);
    ui->tableView_department->setSelectionModel(groupPageSelection_department);

    ui->stackedWidget->setCurrentIndex(11);
    ui->stackedWidget->currentWidget()->setEnabled(true);
}

void MainWindow::on_actMore_triggered() const
{
    ui->stackedWidget->setCurrentIndex(12);
}

void MainWindow::on_actRefresh_triggered()
{
    qDebug() << "心跳query...";
    trayIcon->setToolTip("MagicLight Assistant - 运行中（上次刷新" + QDateTime::currentDateTime().time().toString("hh:mm") + "）");
    int index = ui->stackedWidget->currentIndex();
    switch (index)
    {
    case 0: on_actHome_triggered(); break;
    case 1: on_actMyInfo_triggered(); break;
    case 2: break;
    case 3: on_action_triggered(); break;
    case 4: on_actAttend_triggered(); break;
    case 5: break;
    case 6: on_actUserManager_triggered(); break;
    case 7: on_actAttendManager_triggered(); break;
    case 8: on_actManage_triggered(); break;
    case 9: break;
    case 10: break;
    case 11: on_actGroup_triggered(); break;

    default:
        break;
    }
}

void MainWindow::on_groupPageDptcurrentChanged(const QModelIndex &current, const QModelIndex &previous) const
{
    Q_UNUSED(previous);
    ui->btn_editDpt_cancel->setEnabled(departmentModel->isDirty());
    ui->btn_editDpt_check->setEnabled(departmentModel->isDirty());

    ui->btn_delDpt->setEnabled(current.isValid());
    if(current.row() == 0)
        ui->btn_delDpt->setEnabled(false);  //不能删除默认部门
}

void MainWindow::on_groupPageGroupcurrentChanged(const QModelIndex &current, const QModelIndex &previous) const
{
    Q_UNUSED(previous);
    Q_UNUSED(current);
    ui->btn_editGroup_cancel->setEnabled(groupModel->isDirty());
    ui->btn_editGroup_check->setEnabled(groupModel->isDirty());

    ui->btn_delGroup->setEnabled(current.isValid());
    if(current.row() == 1 || current.row() == 0)
        ui->btn_delGroup->setEnabled(false);  //不能删除默认用户组
}

void MainWindow::on_userManagePagecurrentChanged(const QModelIndex &current, const QModelIndex &previous) const
{
    Q_UNUSED(previous);
    Q_UNUSED(current);
}

void MainWindow::on_userManagePagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QSqlRecord curRecord = userManageModel->record(current.row()), preRecord = userManageModel->record(previous.row());
    
    ui->btn_editUser_check->setEnabled(userManageModel->isDirty());
    ui->btn_editUser_cancel->setEnabled(userManageModel->isDirty());
    
    if(curRecord.value("uid") == "1")
        ui->tableView_userManage->setItemDelegateForRow(current.row(), readOnlyDelegate);   //禁止编辑系统账号
    if (curRecord.value("uid") != "100000" && curRecord.value("uid") != "1" && curRecord.value("uid") != uid)  //避免删除初始用户和当前用户
    {
        ui->btn_delUser->setEnabled(current.isValid());
        ui->btn_banUser->setEnabled(current.isValid());
    }
    else
    {
        ui->btn_delUser->setEnabled(false);
        ui->btn_banUser->setEnabled(false);
    }
    ui->lineEdit_editPwd->setEnabled(curRecord.value("uid") != "1");
    ui->lineEdit_editPwdCheck->setEnabled(curRecord.value("uid") != "1");

    ui->label_userManagePage_uid->setText(curRecord.value("uid").toString());
    if(curRecord.value("name").toString().isEmpty())
        ui->label_userManagePage_name->setText("--");
    else
        ui->label_userManagePage_name->setText(curRecord.value("name").toString());
    ui->label_userManagePage_group->setText(curRecord.value("group_name").toString());    //这里应该填写关联的外键字段名
    ui->label_userManagePage_dpt->setText(curRecord.value("dpt_name").toString());
    if (curRecord.value("user_status").toInt() == 1)
        ui->label_userStatus->setText("账号状态：正常");
    else
        ui->label_userStatus->setText("账号状态：封禁");
    //子线程加载头像
    userManageWork->setCurAvatarUrl(curRecord.value("user_avatar").toString());
    emit userManageGetAvatar();
    
    //密码修改
    if(!ui->lineEdit_editPwd->text().isEmpty())
    {
        if(ui->lineEdit_editPwd->text() == ui->lineEdit_editPwdCheck->text())
        {
            userManageModel->setData(userManageModel->index(previous.row(), userManageModel->fieldIndex("password")), service::pwdEncrypt(ui->lineEdit_editPwd->text()), Qt::EditRole);
            QMessageBox::information(this, "提示", "当前用户（UID：" + preRecord.value("uid").toString()+ "）密码已成功缓存。\n点击确认修改即可生效（请确认UID是否正确*）。\n新密码为：" + ui->lineEdit_editPwd->text(), QMessageBox::Ok);
            ui->btn_editUser_check->setEnabled(userManageModel->isDirty());
            ui->btn_editUser_cancel->setEnabled(userManageModel->isDirty());
            ui->lineEdit_editPwd->clear();
            ui->lineEdit_editPwdCheck->clear();
        }
        else
        {
            QMessageBox::warning(this, "警告", "修改失败，请检查密码填写是否一致。", QMessageBox::Ok);
            ui->lineEdit_editPwdCheck->clear();
        }
    }
}

void MainWindow::on_attendManagePageUserscurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    QSqlRecord curRecord = userManageModel->record(current.row());
    QSqlRecord curAttendRecord;
    curDateTime = QDateTime::currentDateTime();

    ui->btn_attendManage_reAttend->setEnabled(current.isValid());
    ui->btn_attendManage_cancelAttend->setEnabled(current.isValid());

    attendManageModel->setFilter("a_uid='" + curRecord.value("uid").toString() +"'");
    curAttendRecord = attendManageModel->record(0);     //取最新考勤记录
    ui->label_attendManagePage_uid->setText(curRecord.value("uid").toString());

    //获取头像
    attendManageWork->setCurAvatarUrl(curRecord.value("user_avatar").toString());
    emit attendManageGetAvatar();

    if(curAttendRecord.value("today").toString() == curDateTime.date().toString("yyyy-MM-dd"))
    {
        ui->label_attendManagePage_status->setText("已签到");
        ui->label_attendManagePage_beginTime->setText(curAttendRecord.value("begin_date").toString());
        if(curAttendRecord.value("end_date").toString().isEmpty())
            ui->label_attendManagePage_endTime->setText("--");
        else
            ui->label_attendManagePage_endTime->setText(curAttendRecord.value("end_date").toString());
    }
    else
    {
        ui->label_attendManagePage_status->setText("未签到");
        ui->label_attendManagePage_beginTime->setText("--");
        ui->label_attendManagePage_endTime->setText("--");
    }
    if(curRecord.value("name").toString().isEmpty())
        ui->label_attendManagePage_name->setText("--");
    else
        ui->label_attendManagePage_name->setText(curRecord.value("name").toString());
    ui->label_attendManagePage_group->setText(curRecord.value("group_name").toString());    //这里应该填写关联的外键字段名
    ui->label_attendManagePage_dpt->setText(curRecord.value("dpt_name").toString());
}

void MainWindow::on_activityPagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);

    QSqlRecord curRecord = activityModel->record(current.row());
    if (curRecord.value("act_name").toString().isEmpty())
        ui->label_actName->setText("--");
    else
        ui->label_actName->setText(curRecord.value("act_name").toString());
    if (curRecord.value("editUid").toString().isEmpty())
        ui->label_actAuthor->setText("--");
    else
        ui->label_actAuthor->setText(curRecord.value("editUid").toString());
    if (curRecord.value("joinDate").toString().isEmpty())
        ui->label_actJoin->setText("--");
    else
        ui->label_actJoin->setText(curRecord.value("joinDate").toDateTime().toString("yyyy-MM-dd hh:mm"));
    if (curRecord.value("beginDate").toString().isEmpty())
        ui->label_actBegin->setText("--");
    else
        ui->label_actBegin->setText(curRecord.value("beginDate").toDateTime().toString("yyyy-MM-dd hh:mm"));
    if (curRecord.value("endDate").toString().isEmpty())
        ui->label_actEnd->setText("--");
    else
        ui->label_actEnd->setText(curRecord.value("endDate").toDateTime().toString("yyyy-MM-dd hh:mm"));
    if (curRecord.value("act_des").toString().isEmpty())
        ui->textBrowser_actInfo->setText("--");
    else
        ui->textBrowser_actInfo->setText(curRecord.value("act_des").toString());
    ui->label_actScore->setText(curRecord.value("act_score").toString());
}

void MainWindow::on_activityManagePageMemcurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    if (ui->tabWidget_2->currentIndex() != 2)   //切换至报名成员信息页
        ui->tabWidget_2->setCurrentIndex(2);
    emit queryAccount(activityMemModel->record(current.row()).value("actm_uid").toString());
}

void MainWindow::on_myActivityPagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    QSqlRecord curRecord = activityMemModel->record(current.row());
    QString pre_filter = activityModel->filter();
    activityModel->setFilter("act_id=" + curRecord.value("act_id").toString());
    QSqlRecord curActRec = activityModel->record(0);

    if (curActRec.value("act_name").toString().isEmpty())
        ui->label_actName_2->setText("--");
    else
        ui->label_actName_2->setText(curActRec.value("act_name").toString());
    if (curActRec.value("beginDate").toString().isEmpty())
        ui->label_actBegin_2->setText("--");
    else
        ui->label_actBegin_2->setText(curActRec.value("beginDate").toDateTime().toString("yyyy-MM-dd hh:mm"));
    if (curActRec.value("endDate").toString().isEmpty())
        ui->label_actEnd_2->setText("--");
    else
        ui->label_actEnd_2->setText(curActRec.value("endDate").toDateTime().toString("yyyy-MM-dd hh:mm"));
    if (curActRec.value("act_des").toString().isEmpty())
        ui->textBrowser_activityDsc->setText("--");
    else
        ui->textBrowser_activityDsc->setText(curActRec.value("act_des").toString());
    ui->label_actScore_2->setText(curActRec.value("act_score").toString());
    if (curRecord.value("status").toString() == "未录取")
    {
        ui->label_curActStatus->setText("<font color=red>" + curRecord.value("status").toString() + "</font>");
    }else
        ui->label_curActStatus->setText(curRecord.value("status").toString());

    activityModel->setFilter(pre_filter);
}

void MainWindow::on_activityManagePagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    actEditMapper->setCurrentIndex(current.row());  //将映射移动到对应行
    QSqlRecord curRec = activityModel->record(current.row());
    activityMemModel->setFilter("act_id=" + curRec.value("act_id").toString());
    ui->lcdNumber_actMem->display(activityMemModel->rowCount());
    ui->rBtn_actAll->setChecked(true);
}

void MainWindow::on_comboBox_activity_currentIndexChanged(const QString& arg1)
{
    curDateTime = QDateTime::currentDateTime();
    QString dateTime = curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    if (arg1 == "所有活动")
        activityModel->setFilter("");
    else
        activityModel->setFilter("joinDate <= '" + dateTime + "' AND beginDate >= '" + dateTime + "'");
}

void MainWindow::on_comboBox_myAct_currentIndexChanged(const QString& arg1)
{
    curDateTime = QDateTime::currentDateTime();
    QString dateTime = curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    if (arg1 == "所有活动")
        activityMemModel->setFilter("actm_uid=" + uid);
    else if (arg1 == "已录取")
        activityMemModel->setFilter("actm_uid=" + uid + " AND status='已录取'");
    else if (arg1 == "未录取")
        activityMemModel->setFilter("actm_uid=" + uid + " AND status='未录取'");
    else
        activityMemModel->setFilter("actm_uid=" + uid + " AND status='待审核'");
}

void MainWindow::on_btn_addGroup_clicked()
{

    groupModel->insertRow(groupModel->rowCount(),QModelIndex());    //在末尾添加一个记录
    QModelIndex curIndex = groupModel->index(groupModel->rowCount() - 1, 1);    //创建最后一行的ModelIndex
    groupPageSelection_group->clearSelection();//清空选择项
    groupPageSelection_group->setCurrentIndex(curIndex, QItemSelectionModel::Select);//设置刚插入的行为当前选择行

    int currow = curIndex.row(); //获得当前行
    groupModel->setData(groupModel->index(currow, 0), groupModel->rowCount()); //自动生成编号
}

void MainWindow::on_btn_actApprove_clicked()
{
    QSqlRecord curRec = activityMemModel->record(activityMemSelection->currentIndex().row());
    if (curRec.value("actm_id").toString().isEmpty())
    {
        QMessageBox::warning(this, "警告", "请选择正确的报名申请项。");
        return;
    }
    emit approveActivity(curRec.value("actm_id").toString());
}

void MainWindow::on_btn_actReject_clicked()
{
    QSqlRecord curRec = activityMemModel->record(activityMemSelection->currentIndex().row());
    if (curRec.value("actm_id").toString().isEmpty())
    {
        QMessageBox::warning(this, "警告", "请选择正确的报名申请项。");
        return;
    }
    emit rejectActivity(curRec.value("actm_id").toString());
}

void MainWindow::on_btn_actDel_clicked()
{
    QSqlRecord curRec = activityMemModel->record(activityMemSelection->currentIndex().row());
    if (curRec.value("actm_id").toString().isEmpty())
    {
        QMessageBox::warning(this, "警告", "请选择正确的报名申请项。");
        return;
    }
    emit delActivityMem(curRec.value("actm_id").toString());
}

void MainWindow::on_btn_actClearEdit_clicked()
{
    ui->lineEdit_actName->clear();
    ui->textEdit_activity->clear();
    ui->spinBox_actScore->setValue(0);
    ui->dateTimeEdit_actBegin->setDateTime(curDateTime);
    ui->dateTimeEdit_actEnd->setDateTime(curDateTime);
    ui->dateTimeEdit_actJoin->setDateTime(curDateTime);
}

void MainWindow::on_btn_actUpdate_clicked()
{
    bool res = actEditMapper->submit();
    if(res)
        QMessageBox::information(this, "消息", "活动信息更新成功。", QMessageBox::Ok);
    else
        QMessageBox::warning(this, "警告", "活动信息更新失败，错误信息：" + activityModel->lastError().text(), QMessageBox::Ok);
}

void MainWindow::on_btn_actJoin_clicked()
{
    curDateTime = QDateTime::currentDateTime();
    QSqlRecord rec = activityModel->record(myActListSelection->currentIndex().row());
    QString select_id = rec.value("act_id").toString();
    if (rec.value("act_id").toString().isEmpty())
    {
        QMessageBox::warning(this, "错误", "请选择一个有效的活动。");//还要判断活动时间等等，以及是否报名？
        return;
    }
    if (!(rec.value("joinDate").toDateTime() <= curDateTime && curDateTime <= rec.value("beginDate").toDateTime()))
    {
        QMessageBox::warning(this, "错误", "该活动不在报名时间内。");
        return;
    }
    QString pre_filter = activityMemModel->filter();
    activityMemModel->setFilter("actm_uid = " + uid + " AND act_id = " + rec.value("act_id").toString());
    rec = activityMemModel->record(0);
    activityMemModel->setFilter(pre_filter);
    if(!rec.value(0).toString().isEmpty())
    {
        QMessageBox::warning(this, "错误", "你已经报名了该活动，请勿重复报名。");
        return;
    }
    emit applyActivity(select_id, uid);
}

void MainWindow::on_btn_actCancel_clicked()
{
    curDateTime = QDateTime::currentDateTime();
    QSqlRecord rec = activityModel->record(myActListSelection->currentIndex().row()), memRec;
    QString select_id = rec.value("act_id").toString();
    QString pre_filter = activityMemModel->filter();
    activityMemModel->setFilter("actm_uid = " + uid + " AND act_id = " + rec.value("act_id").toString());
    memRec = activityMemModel->record(0);
    activityMemModel->setFilter(pre_filter);
    if (memRec.value(0).toString().isEmpty())
    {
        QMessageBox::warning(this, "错误", "你还没有报名此活动呢，无法取消哦。");
        return;
    }
    if(rec.value("beginDate").toDateTime() <= curDateTime)
    {
        QMessageBox::warning(this, "错误", "该活动已经开始或者结束，无法取消报名哦。");
        return;
    }
    emit cancelActivity(memRec.value("act_id").toString(), uid);
}

void MainWindow::on_btn_actSearch_clicked()
{
    if (ui->comboBox_activity->currentIndex())
        ui->comboBox_activity->setCurrentIndex(0);
    activityModel->setFilter("act_name LIKE '%" + ui->lineEdit_actSearch->text() + "%' OR act_id = '" + ui->lineEdit_actSearch->text() + "'");
}

void MainWindow::on_btn_actSearchClear_clicked()
{
    ui->lineEdit_actSearch->clear();
    activityModel->setFilter("");
}

void MainWindow::on_btn_editGroup_check_clicked()
{
    emit groupManageModelSubmitAll(1);
}

void MainWindow::on_btn_editGroup_cancel_clicked()
{
    groupModel->revertAll();
    removedGroupId.clear();
    ui->btn_editGroup_check->setEnabled(false);
    ui->btn_editGroup_cancel->setEnabled(false);
}

void MainWindow::on_btn_addDpt_clicked()
{
    departmentModel->insertRow(departmentModel->rowCount(), QModelIndex()); //在末尾添加一个记录
    QModelIndex curIndex = departmentModel->index(departmentModel->rowCount() - 1, 1);//创建最后一行的ModelIndex
    groupPageSelection_department->clearSelection();//清空选择项
    groupPageSelection_department->setCurrentIndex(curIndex, QItemSelectionModel::Select);//设置刚插入的行为当前选择行

    int currow = curIndex.row(); //获得当前行
    departmentModel->setData(departmentModel->index(currow, 0), departmentModel->rowCount()); //自动生成编号
}

void MainWindow::on_btn_delDpt_clicked()
{
    QModelIndex curIndex = groupPageSelection_department->currentIndex();//获取当前选择单元格的模型索引
    QSqlRecord curRecord = departmentModel->record(curIndex.row());
    removedDptId = curRecord.value("dpt_id").toString();    //记录已经删除的部门id
    departmentModel->removeRow(curIndex.row()); //删除
    ui->btn_editDpt_check->setEnabled(true);
    ui->btn_editDpt_cancel->setEnabled(true);
}

void MainWindow::on_btn_delGroup_clicked()
{
    QModelIndex curIndex = groupPageSelection_group->currentIndex();//获取当前选择单元格的模型索引
    QSqlRecord curRecord = groupModel->record(curIndex.row());
    removedGroupId = curRecord.value("group_id").toString();    //记录已经删除的用户组id
    groupModel->removeRow(curIndex.row()); //删除
    ui->btn_editGroup_check->setEnabled(true);
    ui->btn_editGroup_cancel->setEnabled(true);

}

void MainWindow::on_btn_editDpt_check_clicked()
{
    emit groupManageModelSubmitAll(0);
}

void MainWindow::on_btn_editDpt_cancel_clicked()
{
    departmentModel->revertAll();
    removedDptId.clear();
    ui->btn_editDpt_check->setEnabled(false);
    ui->btn_editDpt_cancel->setEnabled(false);
}

void MainWindow::on_btn_addUser_clicked()
{
    QMessageBox::information(this, "消息", "新增用户请勿填写UID，程序将自动生成。\n初始密码：123456\n", QMessageBox::Ok);
    userManageModel->insertRow(userManageModel->rowCount(), QModelIndex()); //在末尾添加一个记录
    QModelIndex curIndex = userManageModel->index(userManageModel->rowCount() - 1, 1);//创建最后一行的ModelIndex
    userManagePageSelection->clearSelection();//清空选择项
    userManagePageSelection->setCurrentIndex(curIndex, QItemSelectionModel::Select);//设置刚插入的行为当前选择行

    int currow = curIndex.row(); //获得当前行
    userManageModel->setData(userManageModel->index(currow, userManageModel->fieldIndex("password")), service::pwdEncrypt("123456")); //自动生成密码
    userManageModel->setData(userManageModel->index(currow, userManageModel->fieldIndex("group_name")), 2);
    userManageModel->setData(userManageModel->index(currow, userManageModel->fieldIndex("dpt_name")), 1);
    userManageModel->setData(userManageModel->index(currow, userManageModel->fieldIndex("score")), 0);
    userManageModel->setData(userManageModel->index(currow, userManageModel->fieldIndex("user_status")), 1);
}

void MainWindow::on_btn_delUser_clicked()
{
    QModelIndex curIndex = userManagePageSelection->currentIndex();//获取当前选择单元格的模型索引
    QString uid = userManageModel->record(curIndex.row()).value("uid").toString();
    QString name = userManageModel->record(curIndex.row()).value("name").toString();

    userManageModel->removeRow(curIndex.row()); //删除
    QMessageBox::information(this, "消息", "用户 [" + uid + " " + name + "] 待注销，点击[确认修改]以确认操作。\n注意：该操作会将该用户数据删除，请谨慎操作，如误操作，请点击[取消操作]以撤销。", QMessageBox::Ok);

    ui->btn_editUser_check->setEnabled(true);
    ui->btn_editUser_cancel->setEnabled(true);
}

void MainWindow::on_btn_banUser_clicked()
{
    QModelIndex curIndex = userManagePageSelection->currentIndex();//获取当前选择单元格的模型索引
    QString uid = userManageModel->record(curIndex.row()).value("uid").toString();
    QString name = userManageModel->record(curIndex.row()).value("name").toString();

    if (userManageModel->record(curIndex.row()).value("user_status").toInt() == 1)
    {
        userManageModel->setData(userManageModel->index(curIndex.row(), 10), 0, Qt::EditRole);
        QMessageBox::information(this, "消息", "用户 [" + uid + " " + name + "] 待封禁，点击[确认修改]以确认操作。", QMessageBox::Ok);
    }
    else
    {
        userManageModel->setData(userManageModel->index(curIndex.row(), 10), 1, Qt::EditRole);
        QMessageBox::information(this, "消息", "用户 [" + uid + " " + name + "] 待解封，点击[确认修改]以确认操作。", QMessageBox::Ok);
    }
    ui->btn_editUser_check->setEnabled(true);
    ui->btn_editUser_cancel->setEnabled(true);
}

void MainWindow::on_btn_editUser_check_clicked()
{
    emit userManageModelSubmitAll();
}

void MainWindow::on_btn_editUser_cancel_clicked()
{
    userManageModel->revertAll();
    ui->btn_editUser_check->setEnabled(false);
    ui->btn_editUser_cancel->setEnabled(false);
}

void MainWindow::on_rBtn_man_clicked()
{
    ui->comboBox_group->setCurrentIndex(0);
    ui->comboBox_department->setCurrentIndex(0);
    userManageModel->setFilter("gender='男'");
}

void MainWindow::on_rBtn_woman_clicked()
{
    ui->comboBox_group->setCurrentIndex(0);
    ui->comboBox_department->setCurrentIndex(0);
    userManageModel->setFilter("gender='女'");
}

void MainWindow::on_rBtn_all_clicked()
{
    ui->comboBox_group->setCurrentIndex(0);
    ui->comboBox_department->setCurrentIndex(0);
    userManageModel->setFilter("");
}

void MainWindow::on_rBtn_actAll_clicked()
{
    int idx = -1;
	QString pre_filter = activityMemModel->filter();
    idx = pre_filter.indexOf("AND");
    if (idx != -1)
        pre_filter = pre_filter.mid(0, idx-1);
    if (idx == -1 && pre_filter.indexOf("status") != -1)
        pre_filter.clear();
    activityMemModel->setFilter(pre_filter);
}

void MainWindow::on_rBtn_actFinished_clicked()
{
    int idx = -1;
    QString pre_filter = activityMemModel->filter();
    idx = pre_filter.indexOf("AND");
    if (idx != -1)
        pre_filter = pre_filter.mid(0, idx-1);
    if (idx == -1 && pre_filter.indexOf("status") != -1)
        pre_filter.clear();
    if(pre_filter == "")
        activityMemModel->setFilter("status = '已录取' OR status = '未录取'");
    else
		activityMemModel->setFilter(pre_filter + " AND (status='已录取' OR status='未录取')");
}

void MainWindow::on_rBtn_actPending_clicked()
{
    int idx = -1;
    QString pre_filter = activityMemModel->filter();
    idx = pre_filter.indexOf("AND");
    if (idx != -1)
        pre_filter = pre_filter.mid(0, idx-1);
    if (idx == -1 && pre_filter.indexOf("status") != -1)
        pre_filter.clear();
    if (pre_filter == "")
        activityMemModel->setFilter("status = '待审核'");
    else
        activityMemModel->setFilter(pre_filter + " AND status = '待审核'");
}

void MainWindow::on_btn_userManagePage_search_clicked()
{
    userManageModel->setFilter("uid='" + ui->lineEdit_searchUid->text()+ "' OR name='" + ui->lineEdit_searchUid->text() + "'");
}

void MainWindow::on_btn_userManagePage_recovery_clicked()
{
    userManageModel->setFilter("");
}

void MainWindow::on_comboBox_group_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_dpt(ui->comboBox_group, ui->comboBox_department);
}

void MainWindow::on_comboBox_department_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_dpt(ui->comboBox_group, ui->comboBox_department);
}

void MainWindow::on_btn_userManagePage_recovery_2_clicked()
{
    userManageModel->setFilter("");
    ui->comboBox_group->setCurrentIndex(0);
    ui->comboBox_department->setCurrentIndex(0);
    ui->rBtn_all->setChecked(true);
}

void MainWindow::on_comboBox_group_2_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_dpt(ui->comboBox_group_2, ui->comboBox_department_2);
}

void MainWindow::on_comboBox_department_2_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_dpt(ui->comboBox_group_2, ui->comboBox_department_2);
}

void MainWindow::on_btn_attendManagePage_recovery_clicked()
{
    userManageModel->setFilter("");
    attendManageModel->setFilter("");
    ui->comboBox_group_2->setCurrentIndex(0);
    ui->comboBox_department_2->setCurrentIndex(0);
}

void MainWindow::on_btn_attendManagePage_search_clicked()
{
    userManageModel->setFilter("uid='" + ui->lineEdit_searchUid_attend->text()+ "' OR name='" + ui->lineEdit_searchUid_attend->text() + "'");
}

void MainWindow::on_btn_attendManage_reAttend_clicked()
{
    if(ui->label_attendManagePage_uid->text().isEmpty())
        return;
    if(ui->label_attendManagePage_status->text() == "已签到")
    {
        QMessageBox::warning(this, "消息", "当前用户已签到，无需补签。", QMessageBox::Ok);
        return;
    }
    curDateTime = QDateTime::currentDateTime();
    attendManageModel->insertRow(attendManageModel->rowCount(), QModelIndex()); //在末尾添加一个记录
    QModelIndex curIndex = attendManageModel->index(attendManageModel->rowCount() - 1, 1);//创建最后一行的ModelIndex
    int currow = curIndex.row(); //获得当前行
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("a_uid")), ui->label_attendManagePage_uid->text());
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("today")), curDateTime.date().toString("yyyy-MM-dd"));
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("begin_date")), curDateTime.time().toString("HH:mm:ss"));
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("end_date")), curDateTime.time().toString("HH:mm:ss"));
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("isSupply")), "是");
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("name")), uid);   //这里要填外键关联的字段！
    emit attendManageModelSubmitAll(1);
}

void MainWindow::on_btn_attendManage_cancelAttend_clicked()
{
    curDateTime = QDateTime::currentDateTime();
    QSqlRecord curRecord = attendManageModel->record(0);   //取顶部的数据，因为是按照时间降序排列
    if(ui->label_attendManagePage_status->text() == "未签到")
    {
        QMessageBox::warning(this, "消息", "当前用户未签到，无法退签。", QMessageBox::Ok);
        return;
    }
    //检测即将删除的数据是否与当前用户对应
    if(curRecord.value("a_uid").toString() == ui->label_attendManagePage_uid->text() && curRecord.value("today").toString() == curDateTime.date().toString("yyyy-MM-dd"))
    {
        attendManageModel->removeRow(0); //删除顶部的数据
        emit attendManageModelSubmitAll(0);
    }
    else
        QMessageBox::warning(this, "消息", "数据不匹配，退签失败。", QMessageBox::Ok);

}

void MainWindow::on_btn_attendManagePage_exp_clicked()
{
    ExcelExport expExcel(this);
    QSqlRecord re = attendManageModel->record();
    //导出方式
    int type = -1;
    if(ui->rBtn_attendManagePage_all->isChecked())
        type = 1;
    if(ui->rBtn_attendManagePage_allToday->isChecked())
        type = 2;
    if(ui->rBtn__attendManagePage_curAll->isChecked())
        type = 3;
    curDateTime = QDateTime::currentDateTime();
    QString filePath = QFileDialog::getSaveFileName(this, "导出数据", "考勤数据_" + curDateTime.toString("yyyy-MM-dd_hh-mm-ss"), "Microsoft Excel(*.xlsx)");
    if(expExcel.WriteExcel(filePath, attendManageModel, ui->label_attendManagePage_uid->text(), type))
        QMessageBox::information(this, "消息", "考勤数据已成功导出到：" + filePath, QMessageBox::Ok);
    else
        QMessageBox::warning(this, "消息", "考勤数据导出失败，请检查文件路径。", QMessageBox::Ok);
}

void MainWindow::on_btn_expAttend_clicked()
{
    ExcelExport expExcel(this);
    QSqlRecord re = attendPageModel->record();
    curDateTime = QDateTime::currentDateTime();
    QString filePath = QFileDialog::getSaveFileName(this, "导出数据", "考勤数据_" + curDateTime.toString("yyyy-MM-dd_hh-mm-ss"), "Microsoft Excel(*.xlsx)");
    if(expExcel.WriteExcel(filePath, attendPageModel, ui->label_attendPage_uid->text(), 3))
        QMessageBox::information(this, "消息", "考勤数据已成功导出到：" + filePath, QMessageBox::Ok);
    else
        QMessageBox::warning(this, "消息", "考勤数据导出失败，请检查文件路径。", QMessageBox::Ok);
}

void MainWindow::on_btn_beginAttend_clicked()
{
    if(ui->label_attendPage_status->text() == "已签到")
    {
        QMessageBox::warning(this, "消息", "今天已经在" + ui->label_attendPage_beginTime->text() + "签到过啦~请勿连续签到哦！", QMessageBox::Ok);
        return;
    }
    curDateTime = QDateTime::currentDateTime();
    attendPageModel->insertRow(attendPageModel->rowCount(), QModelIndex()); //在末尾添加一个记录
    QModelIndex curIndex = attendPageModel->index(attendPageModel->rowCount() - 1, 1);//创建最后一行的ModelIndex
    int currow = curIndex.row(); //获得当前行
    attendPageModel->setData(attendPageModel->index(currow, attendPageModel->fieldIndex("a_uid")), uid);
    attendPageModel->setData(attendPageModel->index(currow, attendPageModel->fieldIndex("today")), curDateTime.date().toString("yyyy-MM-dd"));
    attendPageModel->setData(attendPageModel->index(currow, attendPageModel->fieldIndex("begin_date")), curDateTime.time().toString("HH:mm:ss"));
    attendPageModel->setData(attendPageModel->index(currow, attendPageModel->fieldIndex("isSupply")), "否");
    attendPageModel->setData(attendPageModel->index(currow, attendPageModel->fieldIndex("name")), 1);   //这里要填外键关联的字段！

    emit attendPageModelSubmitAll(1);

}

void MainWindow::on_btn_endAttend_clicked()
{
    curDateTime = QDateTime::currentDateTime();

    if(ui->label_attendPage_status->text() != "已签到")
    {
        QMessageBox::warning(this, "消息", "请先签到然后再进行签退哦。", QMessageBox::Ok);
        return;
    }

    if(ui->label_attendPage_endTime->text() != "--")
    {
        QMessageBox::warning(this, "消息", "你已经在" + ui->label_attendPage_endTime->text() + "签退过啦，请勿重复签退哦~", QMessageBox::Ok);
        return;
    }
    emit attendPageModelSubmitAll(0);
}

void MainWindow::on_btn_personalSubmit_clicked()
{
    QString newPwd, newTel, newMail, newAvatar;
    QSqlQuery query;

    if(ui->lineEdit_checkOldPwd->text().isEmpty())
    {
        QMessageBox::warning(this, "消息", "请先验证原密码再提交修改。", QMessageBox::Ok);
        return;
    }
    if(!ui->lineEdit_personalPwd->text().isEmpty())
        newPwd = ui->lineEdit_personalPwd->text();
    if(!ui->lineEdit_personalTel->text().isEmpty())
        newTel = ui->lineEdit_personalTel->text();
    if(!ui->lineEdit_personalMail->text().isEmpty())
        newMail = ui->lineEdit_personalMail->text();
    if (!ui->lineEdit_personalAvatar->text().isEmpty())
        newAvatar = ui->lineEdit_personalAvatar->text();
    if(newPwd != ui->lineEdit_personalPwdCheck->text())
    {
        QMessageBox::warning(this, "警告", "两次密码输入不一致。", QMessageBox::Ok);
        return;
    }
    if(!newPwd.isEmpty() && newPwd.length() < 6)
    {
        QMessageBox::warning(this, "警告", "请输入6~16位的密码以确保安全。", QMessageBox::Ok);
        return;
    }
    emit editPersonalInfo(ui->lineEdit_checkOldPwd->text(), newTel, newMail, newAvatar, newPwd);
}

void MainWindow::on_editPersonalInfoRes(int res)
{
    if(res == -1)
    {
        QMessageBox::warning(this, "消息", "验证原密码验证失败，请检查原密码是否填写正确。", QMessageBox::Ok);
    }
    if(res == 1)
    {
        QMessageBox::warning(this, "消息", "你的个人信息（UID:" + uid + "）已经成功修改。", QMessageBox::Ok);
        on_btn_personalClear_clicked();
        emit startBaseInfoWork();      //刷新个人信息
    }
    if(res == 2)
    {
        QMessageBox::warning(this, "消息", "由于你的（UID:" + uid + "）密码已经修改，账号即将注销，请重新登录。", QMessageBox::Ok);
        on_btn_personalClear_clicked();
        on_actExit_triggered();     //调用注销函数
    }
}

void MainWindow::on_btn_getQQAvatar_clicked()
{
    connect(loadingMovie, &QMovie::frameChanged, this, [=](int tmp)
        {
            Q_UNUSED(tmp);
            ui->btn_getQQAvatar->setIcon(QIcon(loadingMovie->currentPixmap()));
        });

    emit bindQQAvatar(ui->label_info_mail->text());
}

void MainWindow::on_btn_personalClear_clicked()
{
    ui->lineEdit_personalPwd->clear();
    ui->lineEdit_personalPwdCheck->clear();
    ui->lineEdit_personalTel->clear();
    ui->lineEdit_checkOldPwd->clear();
    ui->lineEdit_personalMail->clear();
    ui->lineEdit_personalAvatar->clear();
}

void MainWindow::on_btn_actPush_clicked()
{
    if(ui->lineEdit_actName->text().isEmpty() || ui->textEdit_activity->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, "消息", "请先完善活动相关信息：活动名称，活动内容等。", QMessageBox::Ok);
        return;
    }
    else
    {
        if (!(ui->dateTimeEdit_actJoin->dateTime() < ui->dateTimeEdit_actBegin->dateTime() && ui->dateTimeEdit_actBegin->dateTime() < ui->dateTimeEdit_actEnd->dateTime()))
        {
            QMessageBox::warning(this, "消息", "活动时间不合法，报名时间应早于开始时间且开始时间应早于结束时间。", QMessageBox::Ok);
            return;
        }
        activityModel->insertRow(activityModel->rowCount(), QModelIndex());    //在末尾添加一个记录
        QModelIndex curIndex = activityModel->index(activityModel->rowCount() - 1, 1);    //创建最后一行的ModelIndex
        activitySelection->clearSelection();//清空选择项
        activitySelection->setCurrentIndex(curIndex, QItemSelectionModel::Select);//设置刚插入的行为当前选择行

        int currow = curIndex.row(); //获得当前行
        activityModel->setData(activityModel->index(currow, 1), ui->lineEdit_actName->text()); //填写相应信息
        activityModel->setData(activityModel->index(currow, 2), ui->textEdit_activity->toPlainText());
        activityModel->setData(activityModel->index(currow, 3), ui->dateTimeEdit_actJoin->dateTime().toString("yyyy/MM/dd HH:mm:ss"));
        activityModel->setData(activityModel->index(currow, 4), ui->dateTimeEdit_actBegin->dateTime().toString("yyyy/MM/dd HH:mm:ss"));
        activityModel->setData(activityModel->index(currow, 5), ui->dateTimeEdit_actEnd->dateTime().toString("yyyy/MM/dd HH:mm:ss"));
        activityModel->setData(activityModel->index(currow, 7), ui->spinBox_actScore->value());
        activityModel->setData(activityModel->index(currow, 6), uid);
        emit activityManageModelSubmitAll();
    }
}

void MainWindow::on_btn_actClear_clicked()
{
    QMessageBox::StandardButton res;
    QModelIndex curIndex = activitySelection->currentIndex();//获取当前选择单元格的模型索引
    QSqlRecord curRecord = activityModel->record(curIndex.row());
    if (curRecord.value("act_name").toString() == "")
    {
        QMessageBox::information(this, "提示", "请选择有效的活动。");
        return;
    }
    res = QMessageBox::warning(this, "警告", "确认要删除【" + curRecord.value("act_name").toString() + "】活动吗？", QMessageBox::Yes|QMessageBox::No);
    if (res == QMessageBox::Yes)
        emit delActivity(curRecord.value("act_id").toString());
}

void MainWindow::on_statusChanged(bool status)
{
    qDebug() << "调用on_statusChanged SLOT函数,db:" << status;
    if(!status)
    {
        dbStatus = false;
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("数据库服务状态: " + sqlWork->getTestDb().lastError().text());
    }
    else
    {
        dbStatus = true;
        statusIcon->setPixmap(*statusOKIcon);
        connectStatusLable->setText("数据库服务状态: 已连接数据库");
    }
}

void MainWindow::initToolbar(QSqlRecord rec)
{
    if (rec.value("send_message").toString() == '0')
    {
        actionList[0]->setEnabled(false);
        actionList[0]->setVisible(false);
    }
    else {
        actionList[0]->setEnabled(true);
        actionList[0]->setVisible(true);
    }
    if (rec.value("users_manage").toString() == '0')
    {
        actionList[1]->setEnabled(false);
        actionList[1]->setVisible(false);

    }
    else {
        actionList[1]->setEnabled(true);
        actionList[1]->setVisible(true);
    }
    if (rec.value("attend_manage").toString() == '0')
    {
        actionList[2]->setEnabled(false);
        actionList[2]->setVisible(false);

    }
    else {
        actionList[2]->setEnabled(true);
        actionList[2]->setVisible(true);
    }
    if (rec.value("activity_manage").toString() == '0')
    {
        actionList[3]->setEnabled(false);
        actionList[3]->setVisible(false);

    }
    else {
        actionList[3]->setEnabled(true);
        actionList[3]->setVisible(true);
    }
    if (rec.value("apply_manage").toString() == '0')
    {
        actionList[4]->setEnabled(false);
        actionList[4]->setVisible(false);

    }
    else {
        actionList[4]->setEnabled(true);
        actionList[4]->setVisible(true);
    }
    if (rec.value("applyItem_manage").toString() == '0')
    {
        actionList[5]->setEnabled(false);
        actionList[5]->setVisible(false);

    }
    else {
        actionList[5]->setEnabled(true);
        actionList[5]->setVisible(true);
    }
    if (rec.value("group_manage").toString() == '0')
    {
        actionList[6]->setEnabled(false);
        actionList[6]->setVisible(false);

    }
    else {
        actionList[6]->setEnabled(true);
        actionList[6]->setVisible(true);
    }
    if (rec.value("notice_manage").toString() == '0')
    {
        actionList[7]->setEnabled(false);
        actionList[7]->setVisible(false);

    }
    else {
        actionList[7]->setEnabled(true);
        actionList[7]->setVisible(true);
    }
}

void MainWindow::createActions()
{
    mShowMainAction = new QAction("显示主界面", this);
    connect(mShowMainAction, &QAction::triggered, this, [=]()
		{
            if (this->isHidden())
            {
                this->showMinimized();
                QThread::msleep(150);
                this->showNormal();
                this->setWindowState(Qt::WindowActive);
                this->activateWindow();
            }
            if (this->isMinimized())
                this->showNormal();
		});

    mExitAppAction = new QAction("退出", this);
    connect(mExitAppAction, &QAction::triggered, this, [=]()
		{
            trayIcon->hide();
            this->close();
            QApplication::exit(0);
		});
}

void MainWindow::on_SystemTrayIconClicked(QSystemTrayIcon::ActivationReason action)
{
    switch (action) {
    case QSystemTrayIcon::Trigger:
        if (this->isHidden())
        {
            this->showMinimized();
            QThread::msleep(150);
            this->showNormal();
            this->setWindowState(Qt::WindowActive);
            this->activateWindow();
        }
        if(this->isMinimized())
            this->showNormal();
        break;
    default: break;
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    this->hide();
    event->ignore();
}
