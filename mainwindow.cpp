/***************************************************/
/*                 MagicLitePlanet                 */
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

    infoWidget = new InfoWidget();  //初始化信息窗口
    friendsWidget = new FriendsWidget();    //初始化好友窗口
	friendInfoWidget = new FriendInfoWidget();  //初始化好友资料窗口
	
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

    //webEngine背景透明
    ui->webEngineView->page()->setBackgroundColor(Qt::transparent);
    ui->webEngineView_about->page()->setBackgroundColor(Qt::transparent);
	ui->webEngineView_panel->page()->setBackgroundColor(Qt::transparent);
    //ui->webEngineView->setUrl(QUrl("qrc:/images/loading.html"));
    ui->webEngineView_eCharts->page()->setBackgroundColor(Qt::transparent);
    ui->webEngineView_homeAttendInfo->page()->setBackgroundColor(Qt::transparent);
    ui->webEngineView_workTime->page()->setBackgroundColor(Qt::transparent);

    //用户权限设置（共8个）
    actionList.append(ui->actMessage);
    actionList.append(ui->actUserManager);
    actionList.append(ui->actAttendManager);
    actionList.append(ui->actManage);
    actionList.append(ui->actApplyList);
    actionList.append(ui->actApplyItems);
    actionList.append(ui->actGroup);
    actionList.append(ui->actNoticeManage);

    connectStatusLable = new QLabel("服务状态: 正在连接...");
    connectStatusLable->setMinimumWidth(this->width() - 25);

    userAvatar = new QPixmap(":/images/color_icon/user.svg");
    statusOKIcon = new QPixmap(":/images/color_icon/color-approve.svg"), statusErrorIcon = new QPixmap(":/images/color_icon/color-delete.svg");

	//认证图标
    verifyIcon_1 = new QPixmap(":/images/color_icon/verify.svg");
    verifyIcon_2 = new QPixmap(":/images/color_icon/verify_2.svg");
    verifyNone = new QPixmap(":/images/color_icon/color-delete.svg");

    ui->statusbar->addWidget(statusIcon);   //将状态组件添加至statusBar
    ui->statusbar->addWidget(connectStatusLable);
    ui->stackedWidget->setCurrentIndex(13);  //转到加载首页
    connect(formLoginWindow, SIGNAL(sendData(QString)), this, SLOT(receiveData(QString)));    //接收登录窗口的信号
    readOnlyDelegate = new class readOnlyDelegate(this);    //用于tableView只读

    ui->label_newMsgIcon->setVisible(false);
    ui->label_newMsg->setVisible(false);

    //设置ItemDelegate(用户管理页性别栏)
    comboxList.clear();
    comboxList << "男" << "女";
    comboxDelegateGender.setItems(comboxList, false);
    ui->tableView_userManage->setItemDelegateForColumn(3, &comboxDelegateGender);
    ui->tableView_userManage->setItemDelegate(new QSqlRelationalDelegate(ui->tableView_userManage));

    //加载动画
    avatarLoadMovie = new QMovie(":/images/img/Loading6.gif");
    loadingMovie = new QMovie(":/images/img/Loading4.gif"); //:/images/color_icon/loading.gif
    //ui->label_loading->setMovie(loadingMovie);
    loadingMovie->start();
    avatarLoadMovie->start();

    //心跳query
    refTimer = new QTimer(this);
    connect(refTimer, &QTimer::timeout, this, [=]()  {
            on_actRefresh_triggered();
        });
    msgPushTimer = new QTimer(this);
    connect(msgPushTimer, &QTimer::timeout, this, [=]() {
        if (!isPushing && openChat)  //Push队列处理中时跳过，避免任务堆积
        {
            isPushing = true;
            qDebug() << "正在刷新聊天记录";
            emit startPushMsg(uid, sendToUid, msgStackMax);
        }
        });

    //托盘事件
    trayIconMenu = new QMenu(this);
    createActions();
    trayIconMenu->addAction(mShowMainAction);
    trayIconMenu->addAction(mShowExitAction);
    trayIconMenu->addSeparator();    //分割线
    trayIconMenu->addAction(mExitAppAction);
    trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_SystemTrayIconClicked(QSystemTrayIcon::ActivationReason)));
    QIcon icon(":/images/logo/MagicLightAssistant.png");
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("WePlanet - 运行中");
    trayIcon->setContextMenu(trayIconMenu);
    QFile file(":/qt/qss/qmenu.qss");
    file.open(QFile::ReadOnly);//读取qss文件，设置样式
    if (file.isOpen())
    {
        QString qss = file.readAll();
        trayIconMenu->setStyleSheet(qss);
    }
    file.close();
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
    msgService = new MsgService();
    msgPusherService = new MsgService();
	approvalWork = new ApprovalWork();

    sqlThread = new QThread(), sqlThread_MSG = new QThread(), sqlThread_MSGPUSHER = new QThread(), sqlThread_SECOND = new QThread(), dbThread = new QThread();
    sqlWork->moveToThread(dbThread);
    setBaseInfoWork->moveToThread(sqlThread);
    attendWork->moveToThread(sqlThread);
    userManageWork->moveToThread(sqlThread_SECOND);
    attendManageWork->moveToThread(sqlThread);
    groupManageWork->moveToThread(sqlThread);
    activityManageWork->moveToThread(sqlThread_SECOND);
    posterWork->moveToThread(sqlThread);
    msgService->moveToThread(sqlThread_MSG);
    msgPusherService->moveToThread(sqlThread_MSGPUSHER);
    approvalWork->moveToThread(sqlThread);
    
    //检查更新
    updateSoftWare.moveToThread(sqlThread_SECOND);

    //校验、更新本地时间，本对象中curDateTime即为30分钟更新一次的网络时间
    if (!checkLocalTime())
        disableDynamicItems();
    currentTimeUpdate = new QTimer(this);
    connect(currentTimeUpdate, &QTimer::timeout, this, [=]() {
        static int cnt = 0;
        cnt += 1;
        curDateTime = curDateTime.addSecs(1);
		//qDebug() << "本地时间校验：" << curDateTime.toString("yyyy-MM-dd hh:mm:ss");
        if (cnt > 30 * 60)  //三十分钟校验一次网络时间
        {
            checkLocalTime();
			cnt = 0;
        }
        });
    currentTimeUpdate->start(1000);
        
    connect(this, &MainWindow::beginUpdate, &updateSoftWare, &checkUpdate::parse_UpdateJson);
    connect(&updateSoftWare, &checkUpdate::finished, this, &MainWindow::updateFinished);
    emit beginUpdate();

    //开启数据库连接线程
    dbThread->start();
    sqlThread->start();
    sqlThread_MSG->start();
    sqlThread_MSGPUSHER->start();
    sqlThread_SECOND->start();
    sqlWork->beginThread();
    connect(this, &MainWindow::startDbWork, sqlWork, &SqlWork::working);
    emit startDbWork();

    connect(sqlWork, SIGNAL(newStatus(bool)), this, SLOT(on_statusChanged(bool)));    //数据库心跳验证 5s

    //注册一些信号槽所需
    qRegisterMetaType<QSqlRecord>("QSqlRecord"); 
	qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    qRegisterMetaType<QStack<QByteArray>>("QStack<QByteArray>");

    //初始化Markdown相关
    notice_page = new PreviewPage(this);
    c_channel = new QWebChannel(this);
    m_channel = new QWebChannel(this);

    c_channel->registerObject(QStringLiteral("content"), &c_content);
    m_channel->registerObject(QStringLiteral("content"), &m_content);

    //sqlWork firstFinished信号槽
    connect(sqlWork, &SqlWork::firstFinished, this, [=](){
        sqlWork->stopThread();
		
        //构造model
        attendPageModel = new QSqlRelationalTableModel(this, attendWork->getDB());
        userManageModel = new QSqlRelationalTableModel(this, userManageWork->getDB());
        attendUserModel = new QSqlRelationalTableModel(this, attendManageWork->getDB());
        attendManageModel = new QSqlRelationalTableModel(this, attendManageWork->getDB());
        groupModel = new QSqlTableModel(this, groupManageWork->getDB());
        departmentModel = new QSqlTableModel(this, groupManageWork->getDB());
        activityModel = new QSqlTableModel(this, activityManageWork->getDB());
        activityMemModel = new QSqlTableModel(this, activityManageWork->getDB());
        noticeModel = new QSqlTableModel(this, posterWork->getDB());
        noticeManageModel = new QSqlTableModel(this, posterWork->getDB());

        userManagePageSelection = new QItemSelectionModel(userManageModel);
		attendUserSelection = new QItemSelectionModel(attendUserModel);
        groupPageSelection_group = new QItemSelectionModel(groupModel);
        groupPageSelection_department = new QItemSelectionModel(departmentModel);
        activitySelection = new QItemSelectionModel(activityModel);
        activityMemSelection = new QItemSelectionModel(activityMemModel);
        myActListSelection = new QItemSelectionModel(activityModel);
        myActSelection = new QItemSelectionModel(activityMemModel);
        noticeManageSelection = new QItemSelectionModel(noticeManageModel);
        noticeSelection = new QItemSelectionModel(noticeModel);

        //构造mapper
        actEditMapper = new QDataWidgetMapper(this);
        noticeEditMapper = new QDataWidgetMapper(this);

        //初始化work
        setBaseInfoWork->setUid(uid);
        attendWork->setModel(attendPageModel);
        attendWork->setUid(uid);
        friendsWidget->setUid(uid);
        activityManageWork->setUid(uid);

        userManageWork->setModel(userManageModel);
        userManageWork->setCombox(ui->comboBox_group, ui->comboBox_department);

        attendManageWork->setUserModel(attendUserModel);
        attendManageWork->setAttendModel(attendManageModel);
        attendManageWork->setCombox(ui->comboBox_group_2, ui->comboBox_department_2);

        groupManageWork->setGroupModel(groupModel);
        groupManageWork->setDepartmentModel(departmentModel);

        activityManageWork->setModel(activityModel);
        activityManageWork->setMemberModel(activityMemModel);

        posterWork->setManageModel(noticeManageModel);
        posterWork->setModel(noticeModel);
		
        ui->label_homeStatus->setMovie(loadingMovie);   //首页状态图标
        emit startSetAuth(uid);
        emit startBaseInfoWork();   //等待数据库第一次连接成功后再调用
		ui->label_homeLoading->setVisible(true);
        emit attendHomeChartWorking();
        emit actHomeWorking();
        refTimer->start(5 * 60 * 1000);  //开启心跳query定时器（5分钟心跳）
        msgPushTimer->start(msgPushTime * 1000);
    }, Qt::UniqueConnection);

    connect(this, &MainWindow::get_statistics, setBaseInfoWork, &baseInfoWork::get_statistics);
    connect(this, &MainWindow::loadStatisticsPanel, setBaseInfoWork, &baseInfoWork::loadStatisticsPanel);
    connect(setBaseInfoWork, &baseInfoWork::loadStatisticsPanelFinished, this, &MainWindow::setStatisticsPanel);
	
    //基本信息信号槽
    connect(setBaseInfoWork, &baseInfoWork::baseInfoFinished, this, &MainWindow::setHomePageBaseInfo);
    connect(this, &MainWindow::startBaseInfoWork, setBaseInfoWork, &baseInfoWork::loadBaseInfoWorking);
	connect(this, &MainWindow::loadSystemSettings, setBaseInfoWork, &baseInfoWork::loadSystemSettings);
	connect(setBaseInfoWork, &baseInfoWork::loadSystemSettingsFinished, this, &MainWindow::setSystemSettings);
    connect(this, &MainWindow::saveSystemSettings, setBaseInfoWork, &baseInfoWork::saveSystemSettings);
    connect(setBaseInfoWork, &baseInfoWork::saveSystemSettingsFinished, this, [=](bool res) {
        ui->label_loadingSettings->setMovie(&QMovie());
        if (res)
        {
            ui->label_loadingSettings->setPixmap(QPixmap(":/images/color_icon/approve_3.svg"));
            QMessageBox::information(this, "提示", "系统设置保存成功。");
        }
        else
        {
            ui->label_loadingSettings->setPixmap(QPixmap(":/images/color_icon/color-error.svg"));
            QMessageBox::warning(this, "警告", "系统设置保存失败。");
        }
        });

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
    //首页考勤图表
	connect(this, &MainWindow::attendHomeChartWorking, attendWork, &AttendWork::homeChartWorking);
	connect(attendWork, &AttendWork::homeChartDone, this, [=](QString jsCode) {
		    ui->label_homeLoading->setVisible(false);
		    ui->webEngineView_homeAttendInfo->page()->runJavaScript(jsCode);
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
            ui->label_homeStatus->setMovie(loadingMovie);   //首页状态图标
    		emit startBaseInfoWork();      //刷新个人信息
    		QMessageBox::information(this, "消息", "QQ头像绑定成功，你的头像将会随QQ头像更新。", QMessageBox::Ok);
    	}
        else if(tag == 0)
            QMessageBox::warning(this, "错误", "当前绑定的邮箱并非QQ邮箱，请绑定QQ邮箱后重试。", QMessageBox::Ok);
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
            QMessageBox::information(this, "消息", "签到成功，签到时间：" + curDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\n祝你今天元气满满~", QMessageBox::Ok);
            emit attendWorking();  //刷新信息
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息：\n" + attendPageModel->lastError().text(), QMessageBox::Ok);
    }, Qt::UniqueConnection);
    connect(attendWork, &AttendWork::attendOutDone, this, [=](bool res){
        if(res)
        {
            QMessageBox::information(this, "消息", "签退成功，签退时间：" + curDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\n累了一天了，休息一下吧~", QMessageBox::Ok);
            emit attendWorking();  //刷新信息
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息：\n" + attendPageModel->lastError().text(), QMessageBox::Ok);
    }, Qt::UniqueConnection);

    //用户管理信号槽
    connect(this, &MainWindow::userManageWorking, userManageWork, &UserManageWork::working);
    connect(this, &MainWindow::userManageGetAvatar, userManageWork, &UserManageWork::loadAvatar);
    connect(this, &MainWindow::userManageModelSubmitAll, userManageWork, &UserManageWork::submitAll);
    connect(userManageWork, &UserManageWork::userManageWorkFinished, this, &MainWindow::setUserManagePage);
	connect(this, &MainWindow::updateVerify, userManageWork, &UserManageWork::updateVerify);
	connect(this, &MainWindow::getVerify, userManageWork, &UserManageWork::getVerify);
    connect(userManageWork, &UserManageWork::updateVerifyFinished, this, [=](bool res) {
        if (res)
            QMessageBox::information(this, "消息", "认证系统：操作成功，当前用户认证信息已更新。", QMessageBox::Ok);
        else
			QMessageBox::warning(this, "错误", "认证系统：操作失败，请联系技术支持。", QMessageBox::Ok);
        }, Qt::UniqueConnection);
    connect(userManageWork, &UserManageWork::getVerifyFinished, this, [=](bool res) {
		getVerifyQueue.dequeue();
        if (!getVerifyQueue.isEmpty())
        {
            QString back = getVerifyQueue.back();
            emit getVerify(back);  //若任务堆积，则加载队尾即可
            getVerifyQueue.clear();
            getVerifyQueue.enqueue(back);
        }
        QString verifyType;
        ui->btn_updateVerify->setEnabled(res);
        if (res)
        {
			int verifyTag = userManageWork->getVerifyTag();
            verifyType = userManageWork->getVerifyType();
            ui->btn_updateVerify->setText("更新认证");
            ui->btn_verifyInfo->setEnabled(true);
            ui->btn_delVerify->setEnabled(true);
            if (verifyTag == 1)
                ui->label_verifyIcon_manage->setPixmap(*verifyIcon_1);
            else if (verifyTag == 2)
                ui->label_verifyIcon_manage->setPixmap(*verifyIcon_2);
            else
                ui->label_verifyIcon_manage->setPixmap(*verifyNone);
            ui->label_verifyIcon_manage_main->setPixmap(*ui->label_verifyIcon_manage->pixmap());
            if (verifyTag == -1)
            {
                ui->btn_verifyInfo->setEnabled(false);
                ui->btn_delVerify->setEnabled(false);
                ui->label_verifyIcon_manage_main->setPixmap(QPixmap());
                verifyType = "暂无认证";
                ui->btn_updateVerify->setText("添加认证");
            }
        }
        else
        {
            ui->btn_verifyInfo->setEnabled(false);
            ui->btn_delVerify->setEnabled(false);
            ui->label_verifyIcon_manage->setPixmap(*verifyNone);
            ui->label_verifyIcon_manage_main->setPixmap(*verifyNone);
            ui->btn_updateVerify->setText("暂不可用");
            verifyType = "获取失败";
        }
        ui->label_verifyType_manage->setText(verifyType + QString("（UID:%1）").arg(userManageWork->getUid()));
        infoWidget->setInfoTitle("认证详情：");
        infoWidget->setBoxTitle("认证系统");
        infoWidget->setInfo(userManageWork->getVerifyInfo());
        infoWidget->setInfoIcon(*ui->label_verifyIcon_manage->pixmap());
		
		}, Qt::UniqueConnection);
    connect(userManageWork, &UserManageWork::avatarFinished, this, [=](QPixmap avatar){
        getAvatarQueue.dequeue();
        if (!getAvatarQueue.isEmpty())
        {
            QString back = getAvatarQueue.back();
            userManageWork->setCurAvatarUrl(back);  //若任务堆积，则加载队尾即可
            emit userManageGetAvatar();
            getAvatarQueue.clear();
            getAvatarQueue.enqueue(back);
        }
        if(avatar.isNull())
            ui->userManagePage_avatar->setPixmap(*userAvatar);
        else
            ui->userManagePage_avatar->setPixmap(service::setAvatarStyle(avatar));
    }, Qt::UniqueConnection);
    connect(userManageWork, &UserManageWork::submitAllFinished, this, [=](bool res){
        if(!res)
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息：\n" + userManageModel->lastError().text(),
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
    connect(this, &MainWindow::attendDataOperate, attendManageWork, &AttendManageWork::dataOperate);
    connect(attendManageWork, &AttendManageWork::dataOperateFinished, this, [=](bool res) {
        disconnect(loadingMovie, &QMovie::frameChanged, this, 0);
        ui->btn_oneMonth->setIcon(QIcon());
        ui->btn_threeMonth->setIcon(QIcon());
        ui->btn_removeAll->setIcon(QIcon(":/images/color_icon/color-warning_2.svg"));
        if (res)
        {
            QMessageBox::information(this, "消息", "考勤数据更新完成，页面即将刷新。", QMessageBox::Ok);
            on_actAttendManager_triggered();
        }
        else
            QMessageBox::warning(this, "消息", "考勤数据更新失败，请检查网络或联系管理员。", QMessageBox::Ok);
        }, Qt::UniqueConnection);
    connect(attendManageWork, &AttendManageWork::attendManageWorkFinished, this, &MainWindow::setAttendManagePage);
    connect(attendManageWork, &AttendManageWork::avatarFinished, this, [=](QPixmap avatar){
        getAvatarQueue.dequeue();
        if (!getAvatarQueue.isEmpty())
        {
            QString back = getAvatarQueue.back();
            attendManageWork->setCurAvatarUrl(back);  //若任务堆积，则加载队尾即可
            emit attendManageGetAvatar();
            getAvatarQueue.clear();
            getAvatarQueue.enqueue(back);
        }
        if(avatar.isNull())
            ui->attendManagePage_avatar->setPixmap(*userAvatar);
        else
            ui->attendManagePage_avatar->setPixmap(service::setAvatarStyle(avatar));
    }, Qt::UniqueConnection);
    connect(attendManageWork, &AttendManageWork::submitAddFinished, this, [=](bool res){
        if(res)
        {
            QMessageBox::information(this, "消息", "补签成功，补签时间：" + curDateTime.toString("yyyy-MM-dd hh:mm:ss"),
                                     QMessageBox::Ok);
            ui->label_attendManagePage_status->setText("已签到");
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息：\n" + attendManageModel->lastError().text(),
                                     QMessageBox::Ok);
    });
    connect(attendManageWork, &AttendManageWork::submitDelFinished, this, [=](bool res){
        if(res)
        {
            QMessageBox::information(this, "消息", "当前用户已被成功退签，当日签到数据已经删除。", QMessageBox::Ok);
            ui->label_attendManagePage_status->setText("未签到");
        }
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息：\n" + attendManageModel->lastError().text(),
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
    connect(this, &MainWindow::approveAllActivity, activityManageWork, &ActivityManageWork::m_approveAll);
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
    connect(posterWork, &PosterWork::contentsWorkFinished, this, &MainWindow::setNoticePage);
    connect(this, &MainWindow::poster_statistics, posterWork, &PosterWork::poster_statistics);

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
    //聊天系统信号槽
    connect(this, &MainWindow::loadMsgMemList, msgService, &MsgService::loadMsgMemList);
    connect(msgService, &MsgService::loadMsgMemListFinished, this, &MainWindow::setMsgPage);
    connect(this, &MainWindow::sendMessage, msgService, &MsgService::sendMessage);
    connect(this, &MainWindow::startPushMsg, msgPusherService, &MsgService::pushMessage);
    connect(msgPusherService, &MsgService::pusher, this, &MainWindow::msgPusher);
    connect(msgService, &MsgService::pusher, this, &MainWindow::msgPusher);
    connect(this, &MainWindow::delFriend, msgService, &MsgService::delFriend);
    connect(msgService, &MsgService::sendMessageFinished, [=](bool res) {
        ui->label_send->setMovie(&QMovie());
        if (res)
        {
            ui->label_send->setPixmap(QPixmap(":/images/color_icon/approve_3.svg"));
            curMsgStackCnt++;   //发送成功，当前消息数据量+1
        }
        else
            ui->label_send->setPixmap(QPixmap(":/images/color_icon/approve_2.svg"));
        isSending = false;  //发送完成
        });
    connect(friendsWidget, &FriendsWidget::loadApplyInfoFinished, this, [=]() {
        ui->Msg_ApplyPage->setEnabled(true);
        msgApplyMemBtn->setText(msgApplyMemBtn_Text);
        });
    connect(msgService, &MsgService::delFriendFinished, this, [=](QString res) {
        ui->btn_deleteMsgMem->setEnabled(true);
        ui->btn_deleteMsgMem->setText("删除好友");
        if (res == "OK")
        {
            ui->textBrowser_msgHistory->clear();
            ui->label_msgMemName->setText("--");
            on_btn_newMsgCheacked_clicked();
            QMessageBox::information(this, "消息", QString("已删除好友 [%1] ，请刷新好友列表。").arg(sendToUid), QMessageBox::Ok);
            sendToUid = "-1";
            emit loadMsgMemList(uid);
        }
        else
            QMessageBox::warning(this, "消息", res, QMessageBox::Ok);
        });
    //审批系统信号槽
	connect(this, &MainWindow::loadManagePageApplyItems, approvalWork, &ApprovalWork::getManagePageApplyItems);
	connect(approvalWork, &ApprovalWork::getManagePageApplyItemsFinished, this, &MainWindow::setApplyItemsManagePage);
	connect(this, &MainWindow::loadUserPageApplyItems, approvalWork, &ApprovalWork::getUserPageApplyItems);
    connect(this, &MainWindow::loadApplyFormList, approvalWork, &ApprovalWork::getAllApplyFormList);
    connect(approvalWork, &ApprovalWork::getApplyFormListFinished, this, &MainWindow::setApplyListManagePage);
	connect(approvalWork, &ApprovalWork::getUserPageApplyItemsFinished, this, &MainWindow::setApplyItemsUserPage);
	connect(this, &MainWindow::addOrModifyApplyItem, approvalWork, &ApprovalWork::addOrModifyApplyItem);
	connect(this, &MainWindow::deleteOrSwitchApplyItem, approvalWork, &ApprovalWork::deleteOrSwitchApplyItem);
    connect(this, &MainWindow::submitOrCancelApply, approvalWork, &ApprovalWork::submitOrCancelApply);
    connect(this, &MainWindow::getApplyToken, approvalWork, &ApprovalWork::getApplyToken);
    connect(this, &MainWindow::agreeOrRejectApply, approvalWork, &ApprovalWork::agreeOrRejectApply);
	connect(this, &MainWindow::authApplyToken, approvalWork, &ApprovalWork::authApplyToken);
    connect(approvalWork, &ApprovalWork::authApplyTokenFinished, this, [=](bool res) {
        ui->btn_authApplyToken->setEnabled(true);
        if(!res)
            QMessageBox::warning(this, "消息", "输入的校验码无效，未查询到审批表单。", QMessageBox::Ok);
        else
        {
            QList<QString> list = approvalWork->getAuthApplyTokenResultList();
            ui->lineEdit_applyToken->clear();
            infoWidget->setBoxTitle("审批信息验证系统");
            infoWidget->setInfoTitle("[审批校验码验证通过]");
            infoWidget->setInfoIcon(QPixmap(":/images/color_icon/color-defender.svg"));
			infoWidget->setInfo(QString("申请表单号：%1\n申请人UID：%2\n申请项目：%3\n审批状态：%4\n申请时间：%5").arg(list[0], list[1], list[2], list[3], list[4]));
            infoWidget->showMinimized();
            infoWidget->showNormal();
        }
        });
    connect(approvalWork, &ApprovalWork::agreeOrRejectApplyFinished, this, [=](bool res) {
        if (res)
        {
            ui->textEdit_applyResultText->clear();
            ui->btn_submitApplyResult_argee->setEnabled(false);
            ui->btn_submitApplyResult_reject->setEnabled(false);
            QMessageBox::information(this, "消息", "审核完成，正在刷新数据。", QMessageBox::Ok);
            emit loadApplyFormList(uid);
        }
        else
            QMessageBox::warning(this, "消息", "操作失败，请检查网络或联系管理员。", QMessageBox::Ok);
        });
    connect(approvalWork, &ApprovalWork::getApplyTokenFinished, this, [=](QString token) {
        if(token == "error")
            QMessageBox::warning(this, "消息", "获取失败，请检查网络或联系管理员。", QMessageBox::Ok);
        else
        {
            QClipboard* clipboard = QApplication::clipboard();  //获取系统剪切板指针
            clipboard->setText(token);
            QMessageBox::information(this, "WePlanet 审批系统", QString("校验码：%1\n校验码已复制到你的剪切板。").arg(token), QMessageBox::Ok);
        }
        });
    connect(approvalWork, &ApprovalWork::submitOrCancelApplyFinished, this, [=](bool res) {
        if (res)
        {
			ui->btn_submitApply->setEnabled(false); //提交按钮
			ui->btn_cancelApply->setEnabled(false); //撤销按钮
            ui->btn_setApplyToken->setEnabled(false);  //效验码按钮
            QMessageBox::information(this, "消息", "操作完成，正在刷新数据。", QMessageBox::Ok);
            emit loadUserPageApplyItems(uid);
        }
        else
            QMessageBox::warning(this, "消息", "操作失败，请检查网络或联系管理员。", QMessageBox::Ok);
        });
    connect(approvalWork, &ApprovalWork::addOrModifyApplyItemFinished, this, [=](bool res) {
        if (res)
        {
            isApplyItemEdit = false;
            currentApplyItemID_manage.clear();
			ui->btn_manageApplyPublish->setEnabled(false);
            ui->btn_manageApplyModify->setEnabled(true);
            ui->groupBox_newApply->setEnabled(true);
            ui->groupBox_addApplyOptions->setEnabled(false);
            ui->lineEdit_newApplyTitle->clear();
            QMessageBox::information(this, "消息", "操作完成，正在刷新数据。", QMessageBox::Ok);
            ui->btn_manageApplyPublish->setText("发布申请项");
			emit loadManagePageApplyItems(uid);
        }
        else
			QMessageBox::warning(this, "消息", "操作失败，请检查网络或联系管理员。", QMessageBox::Ok);
    });
    connect(approvalWork, &ApprovalWork::deleteOrSwitchApplyItemFinished, this, [=](bool res) {
        if (res)
        {
            isApplyItemEdit = false;
            currentApplyItemID_manage.clear();
            ui->btn_manageApplyPublish->setEnabled(false);
            ui->btn_manageApplyModify->setEnabled(true);
            ui->groupBox_newApply->setEnabled(true);
            ui->groupBox_addApplyOptions->setEnabled(false);
            ui->btn_manageApplyDelete->setEnabled(false);
            ui->btn_manageApplySwitch->setEnabled(false);
            ui->btn_manageApplyPublish->setText("发布申请项");
            QMessageBox::information(this, "消息", "操作完成，正在刷新数据。", QMessageBox::Ok);
            emit loadManagePageApplyItems(uid);
        }
        else
			QMessageBox::warning(this, "消息", "操作失败，请检查网络或联系管理员。", QMessageBox::Ok);
        });

    //更新HarmonyOS字体
    QFont font;
    int font_Id = QFontDatabase::addApplicationFont(":/src/font/HarmonyOS_Sans_SC_Regular.ttf");
    QStringList fontName = QFontDatabase::applicationFontFamilies(font_Id);
    font.setFamily(fontName.at(0));
    auto listWidget = findChildren<QWidget*>();
    for (auto& widget : listWidget) //遍历所有组件
    {
        font.setBold(widget->font().bold());
        font.setPointSize(widget->font().pointSize());
        widget->setFont(font);
    }
    HarmonyOS_Font = font;
    HarmonyOS_Font_Family = fontName.at(0);

    //好友列表样式
    msgListTips_1 = new QLabel();
    msgListTips_2 = new QLabel();
    msgListTips_1->setMinimumWidth(ui->toolBox_Msg->width());
    msgListTips_1->setMaximumWidth(ui->toolBox_Msg->width());
    msgListTips_1->setAlignment(Qt::AlignHCenter);
    msgListTips_2->setMinimumWidth(ui->toolBox_Msg->width());
    msgListTips_2->setMaximumWidth(ui->toolBox_Msg->width());
    msgListTips_2->setAlignment(Qt::AlignHCenter);
    ui->toolBox_Msg->setStyleSheet(QString("QToolBox::tab,QToolTip{padding-left:5px;border-radius:5px;color:#E7ECF0;background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 #667481,stop:1 #566373)}QToolBox::tab:selected{ background:qlineargradient(spread : pad,x1 : 0,y1 : 0,x2 : 0,y2 : 1,stop : 0 #778899,stop:1 #708090) }QToolButton{font: 10pt \"%1\"; }QLabel{font: 10pt \"%1\";}QToolBox QScrollBar{width:0;height:0}").arg(fontName.at(0)));
    //审批项列表样式
    ui->toolBox_Approval->setStyleSheet(QString("QToolBox::tab,QToolTip{padding-left:5px;border-radius:5px;color:#E7ECF0;background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 #667481,stop:1 #566373)}QToolBox::tab:selected{ background:qlineargradient(spread : pad,x1 : 0,y1 : 0,x2 : 0,y2 : 1,stop : 0 #778899,stop:1 #708090) }QToolButton{font: 10pt \"%1\"; }QLabel{font: 10pt \"%1\";}QToolBox QScrollBar{width:0;height:0}").arg(fontName.at(0)));
    ui->toolBox_Approval_user->setStyleSheet(QString("QToolBox::tab,QToolTip{padding-left:5px;border-radius:5px;color:#E7ECF0;background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 #667481,stop:1 #566373)}QToolBox::tab:selected{ background:qlineargradient(spread : pad,x1 : 0,y1 : 0,x2 : 0,y2 : 1,stop : 0 #778899,stop:1 #708090) }QToolButton{font: 10pt \"%1\"; }QLabel{font: 10pt \"%1\";}QToolBox QScrollBar{width:0;height:0}").arg(fontName.at(0)));
    ui->toolBox_ApprovalList_manage->setStyleSheet(QString("QToolBox::tab,QToolTip{padding-left:5px;border-radius:5px;color:#E7ECF0;background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 #667481,stop:1 #566373)}QToolBox::tab:selected{ background:qlineargradient(spread : pad,x1 : 0,y1 : 0,x2 : 0,y2 : 1,stop : 0 #778899,stop:1 #708090) }QToolButton{font: 10pt \"%1\"; }QLabel{font: 10pt \"%1\";}QToolBox QScrollBar{width:0;height:0}").arg(fontName.at(0)));
    ui->scrollArea_2->setStyleSheet(QString("QToolButton{font: 10pt \"%1\"; }QScrollArea QScrollBar{width:0;height:0}").arg(fontName.at(0)));
    ui->scrollArea_applyForm->setStyleSheet(QString("QTextEdit{font: 10pt \"%1\"; }QScrollArea QScrollBar{width:0;height:0}").arg(fontName.at(0)));
    ui->scrollArea_applyListForm->setStyleSheet(QString("QTextEdit{font: 10pt \"%1\"; }QScrollArea QScrollBar{width:0;height:0}").arg(fontName.at(0)));
        
    //检测开机启动
    ui->checkBox_autoRun->setChecked(isAutoRun(QApplication::applicationFilePath()));

    //绑定快捷键
    ui->btn_sendMsg->setShortcut(QKeySequence(tr("ctrl+return")));   //消息发送

    //系统配置
    config_ini = new QSettings("config.ini", QSettings::IniFormat);
    if (!config_ini->value("/System/MsgPushTime").toBool())
        config_ini->setValue("/System/MsgPushTime", msgPushTime);
    else
        ui->lineEdit_msgPushTime->setText(config_ini->value("/System/MsgPushTime").toString());

    if (!config_ini->value("/System/MsgStackMaxCnt").toBool())
        config_ini->setValue("/System/MsgStackMaxCnt", msgStackMax);
    else
        ui->lineEdit_msgPushMaxCnt->setText(config_ini->value("/System/MsgStackMaxCnt").toString());
}
MainWindow::~MainWindow()
{
    //在此处等待所有线程停止
    if(sqlThread->isRunning())
    {
        sqlThread->quit();
        sqlThread->wait();
    }
    if (sqlThread_MSG->isRunning())
    {
        sqlThread_MSG->quit();
        sqlThread_MSG->wait();
    }
    if (sqlThread_MSGPUSHER->isRunning())
    {
        sqlThread_MSGPUSHER->quit();
        sqlThread_MSGPUSHER->wait();
    }
    if (sqlThread_SECOND->isRunning())
    {
        sqlThread_SECOND->quit();
        sqlThread_SECOND->wait();
    }
    if(dbThread->isRunning())
    {
        sqlWork->stopThread();
        sqlWork->quit();
        dbThread->quit();
        dbThread->wait();
    }
    refTimer->stop();
    msgPushTimer->stop();
    currentTimeUpdate->stop();
    loadingMovie->stop();
    avatarLoadMovie->stop();

    delete config_ini;

    delete infoWidget;
    delete friendsWidget;
    delete friendInfoWidget;
    delete loadingMovie;
    delete avatarLoadMovie;
    delete notice_page;
    delete c_channel;
    delete m_channel;
    //析构所有工作对象和线程
    delete ui;
    delete verifyIcon_1;
    delete verifyIcon_2;
    delete verifyNone;
    delete msgListTips_1;
    delete msgListTips_2;
	
    delete sqlWork;
    delete setBaseInfoWork;
    delete userManageWork;
    delete attendManageWork;
    delete groupManageWork;
    delete activityManageWork;
    delete msgService;
    delete msgPusherService;
    delete approvalWork;

    delete sqlThread;
    delete sqlThread_MSG;
    delete sqlThread_MSGPUSHER;
    delete sqlThread_SECOND;
    delete dbThread;

    delete readOnlyDelegate;
}

void MainWindow::receiveData(QString uid)
{
    this->setEnabled(false);
    this->uid = uid;
    ui->label_home_uid->setText(uid);
    ui->label_info_uid->setText(uid);
    //curDateTime = QDateTime::currentDateTime();
    ui->dateTimeEdit_actJoin->setDateTime(curDateTime);
    ui->dateTimeEdit_actBegin->setDateTime(curDateTime);
    ui->dateTimeEdit_actEnd->setDateTime(curDateTime);

    //重置在线聊天
    initMsgSys();
}

void MainWindow::updateFinished(QString res)
{
    ui->notice->setText(res);
    ui->groupBox_33->setTitle("版本公告（软件版本：Ver " + updateSoftWare.getCurVersion() + "）");
    ui->label_homeVer->setText(updateSoftWare.getCurVersion());
    if (updateSoftWare.getLatestVersion().isEmpty())
        ui->label_LatestVersion->setText("--");
    else
        ui->label_LatestVersion->setText(updateSoftWare.getLatestVersion());
}

/*******************************
 * 功能：开启/关闭 进程开机自动启动
 * 参数：
    1、appPath：需要开启/关闭的自启动软件的“绝对路径”，通常为QApplication::applicationFilePath()
    2、flag：   开启/关闭自启动标志位，1为开启，0为关闭
*******************************/
void MainWindow::setProcessAutoRun(const QString& appPath, bool flag)
{
    QSettings settings(AUTO_RUN, QSettings::NativeFormat);

    //以程序名称作为注册表中的键,根据键获取对应的值（程序路径）
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();    //键-名称

    //如果注册表中的路径和当前程序路径不一样，则表示没有设置自启动或本自启动程序已经更换了路径
    QString oldPath = settings.value(name).toString(); //获取目前的值-绝对路劲
    QString newPath = QDir::toNativeSeparators(appPath);    //toNativeSeparators函数将"/"替换为"\"
    if (flag)
    {
        if (oldPath != newPath)
            settings.setValue(name, newPath);
    }
    else
        settings.remove(name);
}

bool MainWindow::isAutoRun(const QString& appPath)
{
    QSettings settings(AUTO_RUN, QSettings::NativeFormat);
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();
    QString oldPath = settings.value(name).toString();
    QString newPath = QDir::toNativeSeparators(appPath);

    return (settings.contains(name) && newPath == oldPath);
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
    ui->label_home_lastLogin->setText(setBaseInfoWork->getLastLoginTime());

	//认证信息
    if (setBaseInfoWork->getVerifyTag() == -1)
    {
        ui->label_verifyIcon->setPixmap(QPixmap());
        ui->label_verifyType->setText("暂无认证：");
        ui->label_home_verifyInfo->setText("--");
    }
    else
    {
		if(setBaseInfoWork->getVerifyTag() == 1)
            ui->label_verifyIcon->setPixmap(*verifyIcon_1);
        else
            ui->label_verifyIcon->setPixmap(*verifyIcon_2);
        ui->label_verifyType->setText(setBaseInfoWork->getVerifyType() + "：");
        ui->label_home_verifyInfo->setText(setBaseInfoWork->getVerifyInfo());
    }

    ui->label_verifyIcon_2->setPixmap(*ui->label_verifyIcon->pixmap());
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
    if (ui->label_home_lastLogin->text().isEmpty())
        ui->label_home_lastLogin->setText("--");
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
    //curDateTime = QDateTime::currentDateTime();
    ui->label_homePage_attendDate->setText(curDateTime.date().toString("yyyy年MM月dd日"));

    //首页LCD显示工时
    QTime workTime(0, 0, 0, 0);
    QTime beginTime = QTime::fromString(setBaseInfoWork->getBeginTime(), "hh:mm:ss");
    QTime endTime = QTime::fromString(setBaseInfoWork->getEndTime(), "hh:mm:ss");
    workTime = workTime.addSecs(beginTime.secsTo(endTime));
    double workTimeHour = QString::asprintf("%.2f", workTime.hour() + workTime.minute() / 60.0).toDouble();
    
    if(setBaseInfoWork->getAttendToday())
    {
        ui->label_homePage_attendStatus->setText("已签到");
        ui->label_homePage_beginTime->setText(setBaseInfoWork->getBeginTime());
        if (setBaseInfoWork->getEndTime().isNull())
        {
            ui->label_homePage_endTime->setText("--");
            workTime.setHMS(0, 0, 0);
			workTime = workTime.addSecs(beginTime.secsTo(QDateTime::currentDateTime().time()));
            workTimeHour = QString::asprintf("%.2f", workTime.hour() + workTime.minute() / 60.0).toDouble();
            ui->lcdNumber->display(workTimeHour);
        }
        else
        {
            ui->label_homePage_endTime->setText(setBaseInfoWork->getEndTime());
            ui->lcdNumber->display(workTimeHour);
        }
        
    }
    else
    {
        ui->label_homePage_attendStatus->setText("未签到");
        ui->label_homePage_beginTime->setText("--");
        ui->label_homePage_endTime->setText("--");
        ui->lcdNumber->display(0);
    }

	//更新首页状态图标
    ui->label_homeStatus->setMovie(&QMovie());
    if (ui->label_homeVer->text() >= ui->label_LatestVersion->text())
        ui->label_homeStatus->setPixmap(QPixmap(":/images/color_icon/approve_3.svg"));
    else
        ui->label_homeStatus->setPixmap(QPixmap(":/images/color_icon/approve_2.svg"));
	
    sqlWork->beginThread();
}

void MainWindow::setUsersFilter_group(int type, QComboBox *group, QComboBox *department)
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
    if (type == 1)
        userManageModel->setFilter(sqlWhere);
    else
        attendUserModel->setFilter(sqlWhere);
}

void MainWindow::setUsersFilter_dpt(int type, QComboBox *group, QComboBox *department) const
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
    if (type == 1)
        userManageModel->setFilter(sqlWhere);
    else
        attendUserModel->setFilter(sqlWhere);
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
    activityManageWork->setUid(uid);
    setBaseInfoWork->setUid(uid);
    friendsWidget->setUid(uid);
}


void MainWindow::on_actExit_triggered()
{
    QSettings settings("bytecho", "MagicLightAssistant");
    settings.setValue("isAutoLogin", false);    //注销后自动登录失效

    infoWidget->close();
    friendsWidget->close();
    friendInfoWidget->close();

    formLoginWindow = new formLogin();
    refTimer->stop();
    msgPushTimer->stop();

    //重置在线聊天
    initMsgSys();

    trayIcon->hide();
    this->close();
    connect(formLoginWindow, SIGNAL(sendData(QString)), this, SLOT(receiveData(QString)));    //接收登录窗口的信号
    if (formLoginWindow->exec() == QDialog::Accepted)
    {
        formLoginWindow->send();    //发送信号
        resetUID();
        emit startSetAuth(uid);
        ui->label_homeStatus->setMovie(loadingMovie);   //首页状态图标
        emit startBaseInfoWork();
        emit actHomeWorking();
        emit attendHomeChartWorking();
        ui->stackedWidget->setCurrentIndex(13);  //回到首页
        refTimer->start(5 * 60 * 1000);  //开启心跳定时器（5分钟心跳）
        msgPushTimer->start(msgPushTime * 1000);
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
    ui->label_homeStatus->setMovie(loadingMovie);   //首页状态图标
    emit startBaseInfoWork();   //刷新首页数据
    emit attendHomeChartWorking();
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
    ui->stackedWidget->setCurrentIndex(4);
    //curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime());
    
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
    
    //eCharts QChart
    int *workTimeSum = attendWork->getWorkTime();
    QJsonObject seriesObj;
    QJsonArray weekWorkTime = attendWork->getWeekMyWorkTime(), weekAllWorkStatus = attendWork->getWeekAllWorkStatus(), dateArray, weekWorkMem = attendWork->getWeekWorkMem();
    seriesObj.insert("data_yStatus", weekAllWorkStatus);
    seriesObj.insert("data_yTime", weekWorkTime);
    seriesObj.insert("data_yMem", weekWorkMem);
    QString date;
    //curDateTime = QDateTime::currentDateTime();
    curDateTime = curDateTime.addDays(-7);
    for (int i = 7; i >= 1; i--)
    {
        curDateTime = curDateTime.addDays(1);
        date = curDateTime.date().toString("MM.dd");
        dateArray.append(date);
    }
    seriesObj.insert("data_x", dateArray);
    QString jsCode = QString("init(%1, 1)").arg(QString(QJsonDocument(seriesObj).toJson()));
    ui->label_chartMod->setText("我的本周考勤数据");
    ui->webEngineView_eCharts->page()->runJavaScript(jsCode);
    //service::buildAttendChart(ui->chartView_attend, this, ui->label->font(), workTimeSum[0], workTimeSum[1], workTimeSum[2], workTimeSum[3]);  //绘制统计图
    eChartsJsCode = QString(QJsonDocument(seriesObj).toJson());

    QJsonObject myWorkTime[4], myWorkTimeJson;
    myWorkTime[0].insert("value", workTimeSum[0]);
    myWorkTime[0].insert("name", "4h 以下");
    myWorkTime[1].insert("value", workTimeSum[1]);
    myWorkTime[1].insert("name", "4~6h");
    myWorkTime[2].insert("value", workTimeSum[2]);
    myWorkTime[2].insert("name", "6~8h");
    myWorkTime[3].insert("value", workTimeSum[3]);
    myWorkTime[3].insert("name", "8h 以上");
    
	QJsonArray myWorkTimeArray;
    for (auto workTime : myWorkTime)
		myWorkTimeArray.append(workTime);
    QString type = "作息时间很不错~";
    if (workTimeSum[0] > (workTimeSum[1] + workTimeSum[2]))
        type = "需要再勤奋一点哦！";
    else if (workTimeSum[3] > (workTimeSum[1] + workTimeSum[2]))
        type = "要注意休息哦，身体最重要~";
    myWorkTimeJson.insert("type", type);
    myWorkTimeJson.insert("data", myWorkTimeArray);
    jsCode = QString("init(%1)").arg(QString(QJsonDocument(myWorkTimeJson).toJson()));
    ui->webEngineView_workTime->page()->runJavaScript(jsCode);
}

void MainWindow::setStatisticsPanel(int option, int days)
{
    QJsonArray dateArray;
    panel_option = option;
    panel_series_count = 14;
    panel_seriesObj = setBaseInfoWork->getPanelSeriesObj(1), panel_display;
    if (days == 7)
    {
        panel_series_count = 7;
        panel_seriesObj = setBaseInfoWork->getPanelSeriesObj(0);
    }
	//构建显示项
    panel_display.insert("登录请求", false);
	panel_display.insert("注册请求", false);
    panel_display.insert("心跳请求", false);
    panel_display.insert("新增活动", false);
    panel_display.insert("新增动态", false);
    if (panel_option == 0)
    {
        panel_display["登录请求"] = true;
		panel_display["注册请求"] = true;
		panel_display["心跳请求"] = true;
		panel_display["新增活动"] = true;
		panel_display["新增动态"] = true;
        ui->label_panelChartMod->setText("智慧大屏（" + QString::number(panel_series_count) + "天）");
    }
    switch (panel_option)
    {
    case 1:panel_display["登录请求"] = true; ui->label_panelChartMod->setText("登录请求量（" + QString::number(panel_series_count) + "天）"); break;
    case 2:panel_display["注册请求"] = true; ui->label_panelChartMod->setText("注册请求量（" + QString::number(panel_series_count) + "天）"); break;
    case 3:panel_display["心跳请求"] = true; ui->label_panelChartMod->setText("心跳请求量（" + QString::number(panel_series_count) + "天）"); break;
    case 4:panel_display["新增活动"] = true; ui->label_panelChartMod->setText("活动新增量（" + QString::number(panel_series_count) + "天）"); break;
    case 5:panel_display["新增动态"] = true; ui->label_panelChartMod->setText("动态新增量（" + QString::number(panel_series_count) + "天）"); break;
    default:
        break;
    }
    QString date;
    QString jsCode;
    //curDateTime = QDateTime::currentDateTime();
    curDateTime = curDateTime.addDays(-panel_series_count);
    for (int i = panel_series_count; i >= 1; i--)
    {
        curDateTime = curDateTime.addDays(1);
        date = curDateTime.date().toString("MM.dd");
        dateArray.append(date);
    }
    panel_seriesObj.insert("data_x", dateArray);
    jsCode = QString("init(%1, %2)").arg(QString(QJsonDocument(panel_seriesObj).toJson()), QString(QJsonDocument(panel_display).toJson()));
	ui->webEngineView_panel->page()->runJavaScript(jsCode);
	if(ui->stackedWidget->currentIndex() != 16)
        ui->stackedWidget->setCurrentIndex(16);
}

void MainWindow::setSystemSettings()
{
    ui->label_loadingSettings->setMovie(&QMovie());
    if (setBaseInfoWork->getSys_isAnnounceOpen())
        ui->rBtn_announceOpen->setChecked(true);
    else
		ui->rBtn_announceClose->setChecked(true);
    if (setBaseInfoWork->getSys_isTipsAnnounce())
        ui->rBtn_tipsAnnounce->setChecked(true);
    else
        ui->rBtn_warningAnnounce->setChecked(true);
    if (setBaseInfoWork->getSys_isDebugOpen())
        ui->rBtn_debugOpen->setChecked(true);
    else
		ui->rBtn_debugClose->setChecked(true);
    if (setBaseInfoWork->getSys_isOpenChat())
        ui->rBtn_openChat->setChecked(true);
    else
        ui->rBtn_closeChat->setChecked(true);
    ui->textEdit_announcement->setText(setBaseInfoWork->getSys_announcementText());
}

void MainWindow::setMsgPage()
{
    if (!msgService->getIsOpen())
    {
        openChat = false;
        ui->stackedWidget->setCurrentIndex(18); //聊天系统已关闭
        return;
    }else
        openChat = true;
    
    ui->stackedWidget->setCurrentIndex(2);
    //好友列表
    QList<QString> friendList = msgService->getMsgMemList();
    QList<QString> friendNameList = msgService->getMsgMemNameList();
    QList<QPixmap> friendAvatar = msgService->getAvatarList();
    //好友申请列表
    QList<QString> friendApplyList = msgService->getMsgApplyMemList();
    QList<QString> friendApplyNameList = msgService->getMsgApplyMemNameList();
    QList<QPixmap> friendApplyAvatar = msgService->getApplyAvatarList();
    
    //清除提示
    msgListTips_1->setParent(nullptr);
    msgListTips_2->setParent(nullptr);
    if (msgListTipsType == 1)
        ui->Msg_page_vLayout->removeWidget(msgListTips_1);
    else if (msgListTipsType == 2)
        ui->Msg_ApplyPage_vLayout->removeWidget(msgListTips_2);
    msgListTipsType = -1;

    //清除所有widget
    while (ui->Msg_page_vLayout->count())
        ui->Msg_page_vLayout->removeItem(ui->Msg_page_vLayout->itemAt(0));
    while (ui->Msg_ApplyPage_vLayout->count())
        ui->Msg_ApplyPage_vLayout->removeItem(ui->Msg_ApplyPage_vLayout->itemAt(0));
    bool isFriend = false;  //当前聊天对象是否仍为好友
    //好友列表
    for (auto curMem : msgMemberList)
    {
        if (curMem != nullptr)
        {
            msgMemberList.pop_front();
            delete curMem;  //析构上一次的列表
        }
    }
    for (int i = 0; i < friendList.count(); i++)
    {
        QToolButton* msgMember = new QToolButton();
        msgMember->setCheckable(true);
        msgMember->setAutoExclusive(true);
        msgMember->setIcon(friendAvatar[i]);
        msgMember->setIconSize(QSize(50, 50));
        msgMember->setText(QString(" [%1] %2").arg(friendList[i], friendNameList[i]));
        msgMember->setToolTip(friendList[i]);
        msgMember->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        msgMember->setMinimumWidth(ui->toolBox_Msg->width());
        msgMember->setMaximumWidth(ui->toolBox_Msg->width());
        ui->Msg_page_vLayout->addWidget(msgMember);
        msgMemberList.append(msgMember);

        //选中当前聊天对象并判断当前聊天对象是否仍为好友
        if (sendToUid == friendList[i])
        {
            msgMember->setChecked(true);
			isFriend = true;
        }
        
        //按钮事件
        connect(msgMember, &QToolButton::clicked, this, [=]() {
            ui->label_msgMemName->setText("正在和 " + msgMember->text() + " 聊天");
            msgHistoryInfo = QString("<p align='center' style='color:#8d8d8d;font-size:10pt;'>--- 和%1 的聊天记录 ---</p>").arg(msgMember->text());

            curMsgStackCnt = 0;    //切换用户时初始化消息数据量
            msg_contents.clear();   //初始化消息缓存
            sendToUid = msgMember->toolTip();
            emit startPushMsg(uid, sendToUid, msgStackMax);   //获取聊天记录

            ui->textBrowser_msgHistory->clear();
            ui->textBrowser_msgHistory->setCurrentFont(QFont(HarmonyOS_Font.family(), 10));
            ui->textBrowser_msgHistory->append("<br><p align='center' style='color:#8d8d8d;font-size:10pt;'>--- 消息加载中  ---</p><br>");
            });
    }
    ui->Msg_page_vLayout->addStretch(); //添加spacer

    //好友申请列表
    for (auto curMem : msgMemApplyList)
    {
        if (curMem != nullptr)
        {
            msgMemApplyList.pop_front();
            delete curMem;  //析构上一次的列表
        }
    }
    for (int i = 0; i < friendApplyList.count(); i++)
    {
        QToolButton* msgMember = new QToolButton();
        msgMember->setIcon(friendApplyAvatar[i]);
        msgMember->setIconSize(QSize(50, 50));
        msgMember->setText(QString(" [%1] %2").arg(friendApplyList[i], friendApplyNameList[i]));
        msgMember->setToolTip(friendApplyList[i]);
        msgMember->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        msgMember->setMinimumWidth(ui->toolBox_Msg->width());
        msgMember->setMaximumWidth(ui->toolBox_Msg->width());
        ui->Msg_ApplyPage_vLayout->addWidget(msgMember);
        msgMemApplyList.append(msgMember);

        //按钮事件
        connect(msgMember, &QToolButton::clicked, this, [=]() {
            msgApplyMemBtn = msgMember;
            msgApplyMemBtn_Text = msgMember->text();
            msgMember->setText(" 好友申请加载中...");
            ui->Msg_ApplyPage->setEnabled(false);
            friendsWidget->close();
            friendsWidget->loadApply(msgMember->toolTip());
            });
    }
    ui->Msg_ApplyPage_vLayout->addStretch(); //添加spacer
    //当前聊天对象已不是好友
    if (!isFriend)
        initMsgSys();

    //添加提示
    if (friendList.isEmpty())
    {
        msgListTips_1->setText("暂无好友");
        ui->toolBox_Msg->setItemText(0, "我的好友");
        ui->Msg_page_vLayout->addWidget(msgListTips_1);
        ui->Msg_page_vLayout->addStretch(); //添加spacer
        msgListTipsType = 1;
    }
    else {
        ui->toolBox_Msg->setItemText(0, QString("我的好友（共%1名）").arg(friendList.count()));
    }
    if (friendApplyList.isEmpty())
    {
        msgListTips_2->setText("暂无好友申请");
        ui->Msg_ApplyPage_vLayout->addWidget(msgListTips_2);
        ui->Msg_ApplyPage_vLayout->addStretch(); //添加spacer
        msgListTipsType = 2;
		ui->toolBox_Msg->setItemText(1, "好友申请");
    }
    else {
		ui->toolBox_Msg->setItemText(1, QString("好友申请（%1条待审核）").arg(friendApplyList.count()));
        if(!ui->checkBox_noMsgRem->isChecked())
            trayIcon->showMessage("好友验证", QString("你有 %1 条好友申请待审核，请及时查看。").arg(friendApplyList.count()));
    }
}

void MainWindow::setApplyListManagePage()
{
    ui->stackedWidget->setCurrentIndex(9);

    QList<QByteArray> applyFormList = approvalWork->getApplyFormList(); //待审批
    QList<QByteArray> applyFormListDone = approvalWork->getApplyFormListDone(); //已审批

    //清除所有widget
    while (ui->ApprovalList_vLayout->count())
        ui->ApprovalList_vLayout->removeItem(ui->ApprovalList_vLayout->itemAt(0));
    while (ui->ApprovalListDone_vLayout->count())
        ui->ApprovalListDone_vLayout->removeItem(ui->ApprovalListDone_vLayout->itemAt(0));

    static QList<QToolButton*> applyListBtnList;

    for (auto item : applyListBtnList)
    {
        if (item != nullptr)
        {
            applyListBtnList.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    
    //函数对象 auto func = [](xx) { };是一个省去-> ret 的 Lambda 表达式 [ capture ] ( params ) opt -> ret { body; };
    auto setupList = [=](int type, QString apply_id, QString title) -> QToolButton*
    {
        QToolButton* applyListBtn = new QToolButton();
        applyListBtn->setText(QString(" [%1] %2 ").arg(apply_id, title));
        applyListBtn->setMinimumHeight(50);
        applyListBtn->setToolTip(apply_id);
        applyListBtn->setMinimumWidth(ui->toolBox_ApprovalList_manage->width());
        applyListBtn->setMaximumWidth(ui->toolBox_ApprovalList_manage->width());
        applyListBtnList.append(applyListBtn);
        if (type == 0)
            ui->ApprovalList_vLayout->addWidget(applyListBtn);
        else
            ui->ApprovalListDone_vLayout->addWidget(applyListBtn);
        return applyListBtn;
    };

    for (auto form : applyFormList)
    {
        //apply_id, uid, item_id, options, operate_time
        QString apply_id, apply_uid, item_id, options, operate_time, title;
        QDataStream stream(&form, QIODevice::ReadOnly);
        stream >> apply_id >> apply_uid >> item_id >> options >> operate_time;
        title = approvalWork->getApplyItemTitle(item_id);
        connect(setupList(0, apply_id, title), &QToolButton::clicked, this, [=]() {
            currentApplyFormUid = apply_uid;
            currentApplyFormID_manage = apply_id;
            ui->btn_getApplyUserInfo->setEnabled(true);
            ui->label_ApplyListFormTitle->setText(QString("[%1]%2（申请人：%3；申请时间：%4）").arg(apply_id, title, apply_uid, operate_time));
            QString currentTitle, currentOptions;
            QByteArray currentArray = approvalWork->getSimpleApplyItems(item_id);
            QDataStream stream(&currentArray, QIODevice::ReadOnly);
            stream >> currentTitle >> currentOptions;
            QList<QString> currentFormOptionsList = currentOptions.split("$", QString::SkipEmptyParts);
            updateApplyItemOptions(1, currentFormOptionsList);

            ui->textEdit_applyResultText->setEnabled(true);
            ui->btn_submitApplyResult_argee->setEnabled(true);
            ui->btn_submitApplyResult_reject->setEnabled(true);

            QList<QString> applyText = options.split("$", QString::SkipEmptyParts);
            //填入申请信息
            int i = 0;
            for (auto textEdit : applyItemOptions_manage_textEdit)
            {
                if (textEdit != nullptr)
                {
                    textEdit->setReadOnly(true);
                    textEdit->setPlainText(QString("%1：%2").arg(currentFormOptionsList[i], applyText[i]));
                    i++;
                }
            }
            });
    }
    for (auto form : applyFormListDone)
    {
        //apply_id, uid, item_id, options, operate_time
        QString apply_id, apply_uid, item_id, options, operate_time, title;
        QDataStream stream(&form, QIODevice::ReadOnly);
        stream >> apply_id >> apply_uid >> item_id >> options >> operate_time;
        title = approvalWork->getApplyItemTitle(item_id);
        connect(setupList(1, apply_id, title), &QToolButton::clicked, this, [=]() {
            currentApplyFormUid = apply_uid;
            currentApplyFormID_manage = apply_id;
            ui->btn_getApplyUserInfo->setEnabled(true);
            ui->label_ApplyListFormTitle->setText(QString("[%1]%2（申请人：%3 申请时间：%4）").arg(apply_id, title, apply_uid, operate_time));
            QString currentTitle, currentOptions;
            QByteArray currentArray = approvalWork->getSimpleApplyItems(item_id);
            QDataStream stream(&currentArray, QIODevice::ReadOnly);
            stream >> currentTitle >> currentOptions;
            QList<QString> currentFormOptionsList = currentOptions.split("$", QString::SkipEmptyParts);
            updateApplyItemOptions(1, currentFormOptionsList);

            ui->textEdit_applyResultText->setEnabled(false);
            ui->btn_submitApplyResult_argee->setEnabled(false);
            ui->btn_submitApplyResult_reject->setEnabled(false);

            QList<QString> applyText = options.split("$", QString::SkipEmptyParts);
            //填入申请信息
            int i = 0;
            for (auto textEdit : applyItemOptions_manage_textEdit)
            {
                if (textEdit != nullptr)
                {
                    textEdit->setReadOnly(true);
                    textEdit->setPlainText(QString("%1：%2").arg(currentFormOptionsList[i], applyText[i]));
                    i++;
                }
            }
            });
    }
    ui->ApprovalList_vLayout->addStretch();
    ui->ApprovalListDone_vLayout->addStretch();
}

void MainWindow::setApplyItemsManagePage()
{
    ui->stackedWidget->setCurrentIndex(10);
    
    QList<QByteArray> applyItems = approvalWork->getApplyItems();
    
    //清除所有widget
    while (ui->ApprovalItem_page_on_vLayout->count())
        ui->ApprovalItem_page_on_vLayout->removeItem(ui->ApprovalItem_page_on_vLayout->itemAt(0));
    while (ui->ApprovalItem_page_off_vLayout->count())
        ui->ApprovalItem_page_off_vLayout->removeItem(ui->ApprovalItem_page_off_vLayout->itemAt(0));

	static QList<QToolButton*> applyItemBtnList;
    //申请项列表
    for (auto item : applyItemBtnList)
    {
        if (item != nullptr)
        {
            applyItemBtnList.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    for (auto array : applyItems)
    {
        QString applyItemId;
        QString applyItemTitle;
        QString applyItemOptions;
        QString applyItemAuditorList;
        QString applyItemPublisher;
        QString applyItemIsHide;
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream >> applyItemId >> applyItemTitle >> applyItemOptions >> applyItemPublisher >> applyItemAuditorList >> applyItemIsHide;

		applyItemsOptions.insert(applyItemId, applyItemOptions);
        applyItemsAuditorList.insert(applyItemId, applyItemAuditorList);
        applyItemsIsHide.insert(applyItemId, applyItemIsHide);

        QToolButton* applyItemBtn = new QToolButton();
        applyItemBtn->setText(QString(" [%1] %2 ").arg(applyItemId, applyItemTitle));
        applyItemBtn->setMinimumHeight(50);
        applyItemBtn->setToolTip(applyItemId);
        applyItemBtn->setMinimumWidth(ui->toolBox_Approval->width());
        applyItemBtn->setMaximumWidth(ui->toolBox_Approval->width());
        applyItemBtnList.append(applyItemBtn);
        if (applyItemIsHide == "1")
            ui->ApprovalItem_page_off_vLayout->addWidget(applyItemBtn);
        else
            ui->ApprovalItem_page_on_vLayout->addWidget(applyItemBtn);
        
        //按钮事件
        connect(applyItemBtn, &QToolButton::clicked, this, [=]() {
            //切换后，编辑状态取消
            isApplyItemEdit = false;
            ui->groupBox_newApply->setEnabled(true);
            ui->btn_manageApplyPublish->setEnabled(false);
            ui->groupBox_addApplyOptions->setEnabled(false);
            ui->groupBox_addAuditors->setEnabled(false);
            ui->btn_manageApplyDelete->setEnabled(false);
            ui->btn_manageApplySwitch->setEnabled(false);
            ui->btn_manageApplyPublish->setText("发布申请项目");
            ui->btn_manageApplyModify->setEnabled(true);

            currentApplyItemID_manage = applyItemBtn->toolTip();
            ui->label_manageApplyItemTitle->setText(applyItemBtn->text());
            QString applyItemOptions;
            currentApplyItemOptions.clear();
            currentApplyItemAuditorList.clear();
            for (auto option : applyItemsOptions[applyItemBtn->toolTip()].split("$", QString::SkipEmptyParts))
            {
                applyItemOptions += QString("【%1】").arg(option);
                currentApplyItemOptions.push_back(option);
            }
            ui->label_manageApplyOptions->setText(applyItemOptions);
            for (auto auditor : applyItemsAuditorList[applyItemBtn->toolTip()].split(";", QString::SkipEmptyParts))
                currentApplyItemAuditorList.push_back(auditor);
            updateManageApplyItemProcess(applyItemsAuditorList[applyItemBtn->toolTip()].split(";", QString::SkipEmptyParts));   //更新审批流程列表
            });
    }
    ui->ApprovalItem_page_on_vLayout->addStretch(); //添加spacer
    ui->ApprovalItem_page_off_vLayout->addStretch(); //添加spacer

    //审批人员总列表
    //清除所有widget
    while (ui->manageApplyAuditorList_Layout->count())
        ui->manageApplyAuditorList_Layout->removeItem(ui->manageApplyAuditorList_Layout->itemAt(0));
    for (auto item : manageApplyAuditorList)
    {
        if (item != nullptr)
        {
            manageApplyAuditorList.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    for (auto auditor_uid : approvalWork->getAuditorList())
    {
        QToolButton* auditor = new QToolButton();
        auditor->setMinimumSize(140, 40);
        auditor->setText(QString(" [%1] %2 ").arg(auditor_uid, approvalWork->getAuditorName(auditor_uid)));
        auditor->setMinimumHeight(50);
        auditor->setToolTip(auditor_uid);
        auditor->setMinimumWidth(ui->scrollArea_2->width());
        auditor->setMaximumWidth(ui->scrollArea_2->width());
        ui->manageApplyAuditorList_Layout->addWidget(auditor);
        manageApplyAuditorList.push_back(auditor);
        connect(auditor, &QToolButton::clicked, this, [=]() {
            auditor->setEnabled(false);
            currentApplyItemAuditorList.push_back(auditor->toolTip());
            updateManageApplyItemProcess(currentApplyItemAuditorList);
        });
    }
    ui->manageApplyAuditorList_Layout->addStretch(); //添加spacer
}

void MainWindow::setApplyItemsUserPage()
{
    ui->stackedWidget->setCurrentIndex(5);
	QList<QByteArray> applyItems = approvalWork->getApplyItems();
    //清除所有widget
    while (ui->ApprovalItems_vLayout->count())
        ui->ApprovalItems_vLayout->removeItem(ui->ApprovalItems_vLayout->itemAt(0));
    while (ui->ApprovalProcessing_vLayout->count())
        ui->ApprovalProcessing_vLayout->removeItem(ui->ApprovalProcessing_vLayout->itemAt(0));
    while (ui->ApprovalDone_vLayout->count())
        ui->ApprovalDone_vLayout->removeItem(ui->ApprovalDone_vLayout->itemAt(0));
    
    static QList<QToolButton*> applyItemBtnList;
    //申请项列表
    for (auto item : applyItemBtnList)
    {
        if (item != nullptr)
        {
            applyItemBtnList.pop_front();
            delete item;  //析构上一次的列表
        }
    }
	for (auto array : applyItems)
	{
        QString applyItemId;
        QString applyItemTitle;
        QString applyItemOptions;
        QString applyItemAuditorList;
        QString applyItemPublisher;
        QString applyItemIsHide;
        QDataStream stream(&array, QIODevice::ReadOnly);
        stream >> applyItemId >> applyItemTitle >> applyItemOptions >> applyItemPublisher >> applyItemAuditorList >> applyItemIsHide;
        
        applyItemsOptions.insert(applyItemId, applyItemOptions);
        applyItemsAuditorList.insert(applyItemId, applyItemAuditorList);

		QToolButton* applyItemBtn = new QToolButton();
		applyItemBtn->setMinimumHeight(50);
		applyItemBtn->setText(QString(" [%1] %2 ").arg(applyItemId, applyItemTitle));
		applyItemBtn->setToolTip(applyItemId);
		applyItemBtn->setMinimumWidth(ui->toolBox_Approval_user->width());
		applyItemBtn->setMaximumWidth(ui->toolBox_Approval_user->width());
		ui->ApprovalItems_vLayout->addWidget(applyItemBtn);
		applyItemBtnList.append(applyItemBtn);
		connect(applyItemBtn, &QToolButton::clicked, this, [=]() {
			currentApplyItemID_user = applyItemId;
            ui->btn_submitApply->setEnabled(true);  //提交申请按钮
            ui->btn_cancelApply->setEnabled(false);  //撤销申请按钮
			ui->btn_setApplyToken->setEnabled(false);  //效验码按钮
            ui->label_applyStatus->setText("等待提交申请");
		    ui->label_ApplyItemTitle->setText(applyItemBtn->text());
            updateApplyItemProcess(0, "NULL", applyItemsAuditorList[applyItemId].split(";", QString::SkipEmptyParts));
			updateApplyItemOptions(0, applyItemsOptions[applyItemId].split("$", QString::SkipEmptyParts));
			});
	}
    ui->ApprovalItems_vLayout->addStretch(); //添加spacer
	//已提交审批的表单
    QList<QByteArray> applyForms = approvalWork->getApplyForms();
    static QList<QToolButton*> applyFormBtnList;
    for (auto item : applyFormBtnList)
    {
        if (item != nullptr)
        {
            applyFormBtnList.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    for (auto array : applyForms)
    {
		QString applyFormId;
		QString applyFormUid;
		QString applyFormItemId;
		QString applyFormOptions;
		QString applyFormStatus;
		QString applyFormToken;
		QString applyFormTime;
		QDataStream stream(&array, QIODevice::ReadOnly);
		stream >> applyFormId >> applyFormUid >> applyFormItemId >> applyFormOptions >> applyFormStatus >> applyFormToken >> applyFormTime;
		QToolButton* applyFormBtn = new QToolButton();
		applyFormBtn->setMinimumHeight(50);
		applyFormBtn->setText(QString(" [%1] %2 ").arg(applyFormId, approvalWork->getApplyItemTitle(applyFormItemId)));
		applyFormBtn->setToolTip(applyFormId);
		applyFormBtn->setMinimumWidth(ui->toolBox_Approval_user->width());
		applyFormBtn->setMaximumWidth(ui->toolBox_Approval_user->width());
        
        if(applyFormStatus == "0")
		    ui->ApprovalProcessing_vLayout->addWidget(applyFormBtn);    //审核中
        else
            ui->ApprovalDone_vLayout->addWidget(applyFormBtn);  //流程结束
		applyFormBtnList.append(applyFormBtn);
		connect(applyFormBtn, &QToolButton::clicked, this, [=]() {
            currentApplyFormID_user = applyFormId;
            ui->btn_submitApply->setEnabled(false);  //提交申请按钮
            ui->btn_cancelApply->setEnabled(true);  //撤销申请按钮
            ui->btn_setApplyToken->setEnabled(true);  //效验码按钮
		    ui->label_ApplyItemTitle->setText("申请表 " + applyFormBtn->text());
            updateApplyItemProcess(1, applyFormId, applyItemsAuditorList[applyFormItemId].split(";", QString::SkipEmptyParts));
            //申请表单内容
            updateApplyItemOptions(0, applyItemsOptions[applyFormItemId].split("$", QString::SkipEmptyParts));
            QList<QString> applyOptions = applyItemsOptions[applyFormItemId].split("$", QString::SkipEmptyParts);
            QList<QString> applyText = applyFormOptions.split("$", QString::SkipEmptyParts);
            int i = 0;
            for (auto textEdit : applyItemOptions_textEdit)
            {
                if (textEdit != nullptr)
                {
                    textEdit->setReadOnly(true);
                    textEdit->setPlainText(QString("%1：%2").arg(applyOptions[i], applyText[i]));
                    i++;
                }
            }
			});
    }
	ui->ApprovalProcessing_vLayout->addStretch(); //添加spacer
	ui->ApprovalDone_vLayout->addStretch(); //添加spacer
}

void MainWindow::updateManageApplyItemProcess(QList<QString> list)
{
    //清除所有widget
    while (ui->manageApplyProcess_Layout->count())
        ui->manageApplyProcess_Layout->removeItem(ui->manageApplyProcess_Layout->itemAt(0));

    //申请项列表
    for (auto item : manageApplyItemProcess)
    {
        if (item != nullptr)
        {
            manageApplyItemProcess.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    for (auto item : manage_processArrow)
    {
        if (item != nullptr)
        {
            manage_processArrow.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    static int step = 1;
    step = 1;
    for (auto auditor_uid : list)
    {
        QToolButton* process = new QToolButton();
        process->setMinimumSize(140, 40);
        process->setToolTip(QString("审批流程[%1]：[%2]审核").arg(QString::number(step), approvalWork->getAuditorName(auditor_uid)));
        process->setText(QString(" [%1] %2 ").arg(auditor_uid, approvalWork->getAuditorName(auditor_uid)));
        manageApplyItemProcess.append(process);
        ui->manageApplyProcess_Layout->addWidget(process);

        QLabel* arrow = new QLabel();
        arrow->setMinimumSize(45, 40);
        arrow->setMaximumSize(45, 40);
        arrow->setScaledContents(true);
        arrow->setPixmap(QPixmap(":/images/color_icon/arrow_right.svg"));
        manage_processArrow.append(arrow);
        ui->manageApplyProcess_Layout->addWidget(arrow);

        step++;
    }
    if (!list.isEmpty())
    {
        QLabel* icon = new QLabel();
        icon->setMinimumSize(40, 40);
        icon->setMaximumSize(40, 40);
        icon->setScaledContents(true);
        icon->setPixmap(QPixmap(":/images/color_icon/approve_3.svg"));
        manage_processArrow.append(icon);
        ui->manageApplyProcess_Layout->addWidget(icon);
        ui->manageApplyProcess_Layout->addStretch();
    }
}

void MainWindow::updateApplyItemOptions(int type, QList<QString> list)
{
    //清除所有widget
    while (ui->applyForm_Layout->count())
        ui->applyForm_Layout->removeItem(ui->applyForm_Layout->itemAt(0));
    while (ui->applyForm_manage_Layout->count())
        ui->applyForm_manage_Layout->removeItem(ui->applyForm_manage_Layout->itemAt(0));
    //申请项表单填写框
    if (type == 0)
    {
        for (auto item : applyItemOptions_textEdit)
        {
            if (item != nullptr)
            {
                applyItemOptions_textEdit.pop_front();
                delete item;  //析构上一次的列表
                item = nullptr;
            }
        }
    }
    else
    {
        for (auto item : applyItemOptions_manage_textEdit)
        {
            if (item != nullptr)
            {
                applyItemOptions_manage_textEdit.pop_front();
                delete item;  //析构上一次的列表
                item = nullptr;
            }
        }
    }
	for (auto item : list)
	{
		QTextEdit* textEdit = new QTextEdit();
        textEdit->setPlaceholderText(QString("请输入%1...").arg(item));
        if (type == 0)
        {
            textEdit->setMinimumSize(ui->scrollArea_applyForm->width() - 22, 100);
            textEdit->setMaximumSize(ui->scrollArea_applyForm->width() - 22, 100);
            applyItemOptions_textEdit.push_back(textEdit);
            ui->applyForm_Layout->addWidget(textEdit);
        }
        else
        {
            textEdit->setMinimumSize(ui->scrollArea_applyListForm->width() - 22, 100);
            textEdit->setMaximumSize(ui->scrollArea_applyListForm->width() - 22, 100);
            textEdit->setReadOnly(true);
            applyItemOptions_manage_textEdit.push_back(textEdit);
            ui->applyForm_manage_Layout->addWidget(textEdit);
        }
	}
    if (type == 0)
        ui->applyForm_Layout->addStretch();
    else
        ui->applyForm_manage_Layout->addStretch();
}

void MainWindow::updateApplyItemProcess(int type, QString apply_id, QList<QString> list)
{
    if(type == 0)
		ui->groupBox_22->setTitle("当前项目审批流程");
    else
		ui->groupBox_22->setTitle("当前申请审批进度（点击具体流程可查看审核意见）");
    //清除所有widget
    while (ui->ApplyProcess_Layout->count())
        ui->ApplyProcess_Layout->removeItem(ui->ApplyProcess_Layout->itemAt(0));

    //申请项列表
    bool isReject = false;
    for (auto item : applyItemProcess)
    {
        if (item != nullptr)
        {
            applyItemProcess.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    for (auto item : user_processArrow)
    {
        if (item != nullptr)
        {
            user_processArrow.pop_front();
            delete item;  //析构上一次的列表
        }
    }
    static int step = 1;
    step = 1;
    QList<QByteArray> processResultList = approvalWork->getCurrentApplyProcess(apply_id);   //审核结果
    for (auto auditor_uid : list)
    {
        QToolButton* process = new QToolButton();
        process->setMinimumSize(140, 40);
        process->setToolTip(QString("审批流程[%1]：[%2]审核").arg(QString::number(step), approvalWork->getAuditorName(auditor_uid)));
        process->setText(QString(" [%1] %2 ").arg(auditor_uid, approvalWork->getAuditorName(auditor_uid)));
        applyItemProcess.append(process);
        
        if (type == 1)
        {
            process->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            process->setIcon(QIcon(":/images/color_icon/approve.svg"));  //等待审核
            ui->label_applyStatus->setText(QString("等待[%1]审核").arg(approvalWork->getAuditorName(auditor_uid)));
            if(isReject)
                process->setIcon(QIcon(":/images/color_icon/color-delete.svg"));  //已终止
        }
		if (type == 1 && step <= processResultList.count())
        {
            QByteArray array = processResultList[step - 1];
            QDataStream stream(&array, QIODevice::ReadOnly);
            QString result, result_text, operate_time;
			stream >> result >> result_text >> operate_time;
            if (result == "0")
            {
				isReject = true;
                process->setIcon(QIcon(":/images/color_icon/approve_2.svg"));  //不通过
            }
            else
            {
                isReject = false;
                process->setIcon(QIcon(":/images/color_icon/approve_3.svg"));  //通过
            }
            //点击事件，输出审核意见
            connect(process, &QToolButton::clicked, this, [=]() {
                infoWidget->setBoxTitle("审核结果");
                QString res;
                if (isReject)
                {
                    res = "驳回";
                    infoWidget->setInfoIcon(QPixmap(":/images/color_icon/approve_2.svg"));
                }
                else
                {
					res = "通过";
					infoWidget->setInfoIcon(QPixmap(":/images/color_icon/approve_3.svg"));
                }
                infoWidget->setInfoTitle(QString("[%1]%2：%3").arg(auditor_uid, approvalWork->getAuditorName(auditor_uid), res));
                infoWidget->setInfo(QString("审核意见：%2\n审核时间：%1").arg(operate_time, result_text));
                infoWidget->showMinimized();
                infoWidget->showNormal();
                });
        }      
        ui->ApplyProcess_Layout->addWidget(process);

        QLabel* arrow = new QLabel();
        arrow->setMinimumSize(45, 40);
        arrow->setMaximumSize(45, 40);
        arrow->setScaledContents(true);
        arrow->setPixmap(QPixmap(":/images/color_icon/arrow_right.svg"));
        user_processArrow.append(arrow);
        ui->ApplyProcess_Layout->addWidget(arrow);

        step++;
    }
    if (step - 1 == processResultList.count() && isReject == false)
        ui->label_applyStatus->setText("审批流程已通过");
    else if(isReject)
        ui->label_applyStatus->setText("审批流程已终止");
    if (!list.isEmpty())
    {
        QLabel* icon = new QLabel();
        icon->setMinimumSize(40, 40);
        icon->setMaximumSize(40, 40);
        icon->setScaledContents(true);
		if (isReject)
			icon->setPixmap(QPixmap(":/images/color_icon/color-delete.svg"));
		else
			icon->setPixmap(QPixmap(":/images/color_icon/approve_3.svg"));
        user_processArrow.append(icon);
        ui->ApplyProcess_Layout->addWidget(icon);
        ui->ApplyProcess_Layout->addStretch();
    }
}

void MainWindow::msgPusher(QStack<QByteArray> msgStack)
{
    isPushing = false;  //消息推送队列已经处理完成

    //qDebug() << "刷新消息cur:" << curMsgStackCnt << "stack:" << msgPusherService->getMsgStackCnt(sendToUid);
    if (curMsgStackCnt < msgPusherService->getMsgStackCnt(sendToUid))  //有新消息
    {
        if (curMsgStackCnt != 0)
        {
            ui->label_newMsg->setText("<font color=red>" + ui->label_newMsg->text() + "</font>");
            ui->label_newMsgIcon->setVisible(true);
            ui->label_newMsg->setVisible(true);
            ui->btn_newMsgCheacked->setEnabled(true);
        }
        if (curMsgStackCnt != 0 && !ui->checkBox_noMsgRem->isChecked())
            trayIcon->showMessage("消息提醒", QString("你有一条来自[%1]的新消息~").arg(sendToUid));
        curMsgStackCnt = msgPusherService->getMsgStackCnt(sendToUid);
    }

    //添加聊得火热
    if (msgPusherService->getMsgStackCnt(sendToUid) >= 30 && ui->label_msgMemName->text().indexOf(" 🔥 ") == -1)
        ui->label_msgMemName->setText(ui->label_msgMemName->text() + " 🔥 ");

    if (curMsgStackCnt > msgPusherService->getMsgStackCnt(sendToUid))  //消息历史过旧，才会推送新消息
        return;
    if (msgPusherService->getPreviousPushUid() != msgPusherService->getPushingUid()) //如果已切换用户，则跳过此次push
        return;
    if (isSending)  //如果正在发送消息，则跳过此次push
        return;

    QString from_uid, from_name, to_uid, to_name, msgText, send_time;
    
    msgBeforePos = ui->textBrowser_msgHistory->verticalScrollBar()->value();   //滚动条位置
    bool atEnd = ui->textBrowser_msgHistory->verticalScrollBar()->maximum() <= msgBeforePos;  //是否在底部
    ui->textBrowser_msgHistory->clear();

    if (msgStack.isEmpty())
    {
        ui->textBrowser_msgHistory->append("<br><p align='center' style='color:#8d8d8d;font-size:10pt;'>--- 当前暂无聊天记录 ---</p><br>");
        return;
    }
    msg_contents.clear();
    while (!msgStack.isEmpty())
    {
        QDataStream stream(&msgStack.pop(), QIODevice::ReadOnly);   //消息出栈
        stream >> from_uid >> from_name >> to_uid >> to_name >> msgText >> send_time;
        QDateTime sendDate = QDateTime::fromString(send_time, "yyyy-MM-dd hh:mm:ss");

        if (sendDate.date() == curDateTime.date())
            send_time = sendDate.time().toString("hh:mm:ss");   //若时间为当前，则简化显示
        if (from_uid == uid)
        {
            msg_contents += QString("<p align='right' style='margin-right:15px;color:#8d8d8d;font-family:%4;font-size:10pt;'>%2 %3</p>").arg(from_name, send_time, HarmonyOS_Font_Family);
            msg_contents += QString("<p align='right' style='margin-top:20px; margin-bottom:20px;margin-right:15px;font-size:12pt;'>%1 📨 </p>").arg(msgText);
        }
        else
        {
            msg_contents += QString("<p align='left' style='margin-left:15px;color:#8d8d8d;font-family:%4;font-size:10pt;'>[%1] %2 %3</p>").arg(from_uid, from_name, send_time, HarmonyOS_Font_Family);
            msg_contents += QString("<p align='left' style='margin-top:20px; margin-bottom:20px;margin-left:15px;font-size:12pt;'> 📣 %1</p>").arg(msgText);
        }
    }
    ui->textBrowser_msgHistory->insertHtml(QString("%1%2<p>").arg(msgHistoryInfo, msg_contents));

    //修复滚动条最大高度可能不正确的问题(未研究QT源码，暂不清楚误差产生原因...)
    int pageStep = ui->textBrowser_msgHistory->verticalScrollBar()->pageStep();
    int documentHeight = ui->textBrowser_msgHistory->document()->size().height();
	int scrollBarMax = ui->textBrowser_msgHistory->verticalScrollBar()->maximum();
    if (documentHeight - pageStep > scrollBarMax)
    {
        ui->textBrowser_msgHistory->verticalScrollBar()->setMaximum(documentHeight - pageStep);
        scrollBarMax = documentHeight - pageStep;
    }
    
    if (!atEnd)
        ui->textBrowser_msgHistory->verticalScrollBar()->setValue(msgBeforePos);  //滚动条不在末尾，则恢复原位置，这里也有偶尔会下移一段距离的问题
    else
        ui->textBrowser_msgHistory->verticalScrollBar()->setValue(scrollBarMax);
}

void MainWindow::initMsgSys()
{
    sendToUid = "-1";
    isPushing = false;
    curMsgStackCnt = 0;
    msgHistoryInfo.clear();
    msg_contents.clear();
    ui->textBrowser_msgHistory->clear();
    ui->label_msgMemName->setText("--");
    on_btn_newMsgCheacked_clicked();
}

bool MainWindow::checkLocalTime()
{
    qint32 webTimeSinceEpoch = service::getWebTime();
    curDateTime = QDateTime::fromSecsSinceEpoch(webTimeSinceEpoch); //获取网络时间
    if (webTimeSinceEpoch == -1)
    {
        QMessageBox::warning(this, "时间误差警告", "获取服务器时间失败，请检查网络连接。\n考勤、活动、畅聊等已被禁用，请前往【设置】页面重新验证时间以启动部分项。");
        return false;
    }
	QDateTime webTime = QDateTime::fromSecsSinceEpoch(webTimeSinceEpoch);   //获取网络时间
	QDateTime localTime = QDateTime::currentDateTime();   //获取本地时间
	double marginMinutes = localTime.secsTo(webTime) / 60.0;    //计算时间差
	double marginSeconds = curDateTime.secsTo(webTime);    //计算内存中的时间差
    
    if(marginSeconds > 5 || marginSeconds < -5)
        curDateTime = QDateTime::fromSecsSinceEpoch(webTimeSinceEpoch); //更新网络时间
	if (marginMinutes > 3 || marginMinutes < -3)
	{
		QMessageBox::warning(this, "时间误差警告", "本地时间与Windows服务器时间的误差超出范围。\n考勤、活动、畅聊等已被禁用，请检查本地时间后前往【设置】页面重新验证时间以启动部分项。");
		return false;
	}
    return true;
}

void MainWindow::disableDynamicItems()
{
    ui->btn_actJoin->setEnabled(false);
    ui->btn_actCancel->setEnabled(false);
    ui->btn_beginAttend->setEnabled(false);
    ui->btn_endAttend->setEnabled(false);
    ui->btn_sendMsg->setEnabled(false);
}

void MainWindow::on_actApply_triggered()
{
    //ui->stackedWidget->setCurrentIndex(5);
   if (ui->stackedWidget->currentIndex() == 13)
        return;
   ui->stackedWidget->setCurrentIndex(13);
   emit loadUserPageApplyItems(uid);
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
    /*
    //当前项变化时触发currentChanged信号
    connect(userManagePageSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                this, SLOT(on_userManagePagecurrentChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    */
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
    ui->tableView_attendUsers->setModel(attendUserModel);
    ui->tableView_attendUsers->hideColumn(1);  //隐藏密码
    ui->tableView_attendUsers->hideColumn(8);  //头像地址
    ui->tableView_attendUsers->hideColumn(9);  //学时
    ui->tableView_attendUsers->hideColumn(10); //用户状态
    
    ui->tableView_attendUsers->setSelectionModel(attendUserSelection);
    
    //当前行变化时触发currentRowChanged信号
    connect(attendUserSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
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

    //activityMemModel->setFilter("act_id='--'");
    for (int i = 0; i < activityMemModel->rowCount(); i++)
            ui->tableView_actMember->hideRow(i);
    
    activityModel->setFilter("editUid=" + uid);     //仅能管理自己发布的活动
    ui->stackedWidget->setCurrentIndex(8);
    ui->stackedWidget->currentWidget()->setEnabled(true);
}

void MainWindow::setNoticeManagePage()
{
    ui->tableView_mContents->setModel(noticeManageModel);
    noticeEditMapper->setModel(noticeManageModel);
    noticeEditMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    ui->tableView_mContents->hideColumn(2);  //隐藏一些列
    ui->tableView_mContents->hideColumn(3);
    ui->tableView_mContents->hideColumn(4);  
    ui->tableView_mContents->hideColumn(5);
    ui->tableView_mContents->hideColumn(6);
    ui->tableView_mContents->hideColumn(7);
    ui->tableView_mContents->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//填充整个view
    //SelectionModel
    ui->tableView_mContents->setSelectionModel(noticeManageSelection);
    connect(noticeManageSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
        this, SLOT(on_noticeManagePagecurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    //Mapper
    noticeEditMapper->setModel(noticeManageModel);
    noticeEditMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    noticeEditMapper->addMapping(ui->lineEdit_manageContents, 1);
    noticeEditMapper->addMapping(ui->contents_editor, 2);
    noticeEditMapper->addMapping(ui->checkBox_mCisHide, 6);
    noticeEditMapper->addMapping(ui->rBtn_mCPost, 5);

    noticeManageModel->setFilter("author_id=" + uid);
    ui->stackedWidget->setCurrentIndex(14);
}

void MainWindow::setNoticePage()
{
    ui->stackedWidget->setCurrentIndex(15);

    ui->tableView_contents->setModel(noticeModel);

    ui->tableView_contents->hideColumn(2);  //隐藏一些列
    ui->tableView_contents->hideColumn(3);
    ui->tableView_contents->hideColumn(4);
    ui->tableView_contents->hideColumn(5);
    ui->tableView_contents->hideColumn(6);
    ui->tableView_contents->hideColumn(7);
    ui->tableView_contents->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//填充整个view

    //SelectionModel
    ui->tableView_contents->setSelectionModel(noticeSelection);
    connect(noticeSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
        this, SLOT(on_myNoticePagecurrentRowChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
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
    if (getAvatarQueue.isEmpty())
    {
        userManageWork->setCurAvatarUrl(rec.value("user_avatar").toString());
        emit userManageGetAvatar();
    }
    getAvatarQueue.enqueue(rec.value("user_avatar").toString());   //加载项入栈
}

void MainWindow::on_btn_manageApplyAddApply_clicked()
{
    if (ui->lineEdit_newApplyTitle->text().isEmpty())
        return;
    QDataStream stream(&newApplyItem, QIODevice::WriteOnly);
    stream << ui->lineEdit_newApplyTitle->text();
    ui->btn_manageApplyPublish->setEnabled(true);
    ui->groupBox_addAuditors->setEnabled(true);
    ui->groupBox_addApplyOptions->setEnabled(true);
    
    ui->label_manageApplyItemTitle->setText(ui->lineEdit_newApplyTitle->text());
    ui->label_manageApplyOptions->setText("--");
    on_btn_reManageApplyProcess_clicked();  //重置审批流程
    on_btn_reManageApplyOptions_clicked();  //重置审批表单
}

void MainWindow::on_btn_manageApplyPublish_clicked()
{
	if (currentApplyItemAuditorList.isEmpty() || currentApplyItemOptions.isEmpty())
		return;
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    QString applyItemAuditorList, applyItemOptions; //格式化数据
	for (auto auditor : currentApplyItemAuditorList)
		applyItemAuditorList += auditor + ";";
	for (auto option : currentApplyItemOptions)
		applyItemOptions += option + "$";
    if (isApplyItemEdit)
    {
        approvalWork->setModifyItemID(currentApplyItemID_manage);  //正在编辑的编号
        stream << QString("NULL") << applyItemOptions << uid << applyItemAuditorList << QString("NULL");
        emit addOrModifyApplyItem(1, array);
    }
    else
    {
        if (ui->lineEdit_newApplyTitle->text().isEmpty())
            return;
        stream << ui->lineEdit_newApplyTitle->text() << applyItemOptions << uid << applyItemAuditorList << QString("0");
        emit addOrModifyApplyItem(0, array);
    }
}

void MainWindow::on_btn_manageApplyDelete_clicked()
{
    const QMessageBox::StandardButton res = QMessageBox::warning(this, "警告", QString("确认要删除申请项目%1吗？").arg(ui->label_manageApplyItemTitle->text()), QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
        emit deleteOrSwitchApplyItem(0, currentApplyItemID_manage);
}

void MainWindow::on_btn_manageApplySwitch_clicked()
{
    emit deleteOrSwitchApplyItem(1, currentApplyItemID_manage);
}

void MainWindow::on_btn_submitApply_clicked()
{
    QString formText;
	for (auto textEdit : applyItemOptions_textEdit)
	{
		if (textEdit->toPlainText().isEmpty() || textEdit->toPlainText().indexOf("$") != -1)
		{
			QMessageBox::warning(this, "错误", "请填写所有表单项并检查是否有非法字符。");
			return;
		}
		formText.push_back(textEdit->toPlainText() + "$");
	}
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
	stream << uid << currentApplyItemID_user << formText << curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    
	emit submitOrCancelApply(1, "NULL", array);
}

void MainWindow::on_btn_cancelApply_clicked()
{
    const QMessageBox::StandardButton res = QMessageBox::warning(this, "警告", QString("确认要撤销%1吗？你的申请表将会被删除。").arg(ui->label_ApplyItemTitle->text()), QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
        emit submitOrCancelApply(0, currentApplyFormID_user);
}

void MainWindow::on_btn_setApplyToken_clicked()
{
    if (currentApplyFormID_user.isEmpty())
        return;
    emit getApplyToken(currentApplyFormID_user);
}

void MainWindow::on_btn_getApplyUserInfo_clicked()
{
    friendInfoWidget->setTitle("申请人信息");
    friendInfoWidget->hideButton(true);
    friendInfoWidget->showMinimized();
    friendInfoWidget->setUid(currentApplyFormUid);
    friendInfoWidget->showNormal();
}

void MainWindow::on_btn_submitApplyResult_argee_clicked()
{
    if (ui->textEdit_applyResultText->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, "提示", "你还没有填写审核意见。", QMessageBox::Ok);
        return;
    }
    emit agreeOrRejectApply(currentApplyFormID_manage, uid, QString("1"), ui->textEdit_applyResultText->toPlainText());
}

void MainWindow::on_btn_submitApplyResult_reject_clicked()
{
    if (ui->textEdit_applyResultText->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, "提示", "你还没有填写审核意见。", QMessageBox::Ok);
        return;
    }
    emit agreeOrRejectApply(currentApplyFormID_manage, uid, QString("0"), ui->textEdit_applyResultText->toPlainText());
}

void MainWindow::on_btn_authApplyToken_clicked()
{
	if (ui->lineEdit_applyToken->text().isEmpty())
    {
        QMessageBox::warning(this, "提示", "你还没有填写审批校验码。", QMessageBox::Ok);
        return;
    }
	emit authApplyToken(ui->lineEdit_applyToken->text());
    ui->btn_authApplyToken->setEnabled(false);
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
    //activityManageWork->setUid(uid);
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
    // curDateTime = QDateTime::currentDateTime();
    // ui->dateTimeEdit_actBegin->setDateTime(curDateTime);
    // ui->dateTimeEdit_actEnd->setDateTime(curDateTime);
    // ui->dateTimeEdit_actJoin->setDateTime(curDateTime);

    ui->tableView_actList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_actList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_actMember->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_actMember->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView_actMember->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    //填充整个view

    ui->tableView_actList->setItemDelegateForColumn(0, readOnlyDelegate);
    ui->tableView_actList->setItemDelegateForColumn(6, readOnlyDelegate);
    ui->tableView_actMember->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::on_actMessage_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    emit loadMsgMemList(uid);
}

void MainWindow::on_actNotice_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    posterWork->setWorkType(0);
    emit posterWorking();

    //初始化Markdown解析
    ui->Contents_preview->setPage(notice_page);
    
    connect(noticeSelection, &QItemSelectionModel::currentRowChanged,
        this, [=](const QModelIndex& current, const QModelIndex& previous) {
            Q_UNUSED(previous);
            QSqlRecord curRec = noticeModel->record(current.row());
            c_content.setText(curRec.value("text").toString());
        }, Qt::UniqueConnection);
    notice_page->setWebChannel(c_channel);

    ui->Contents_preview->setUrl(QUrl("qrc:/images/index.html"));

    ui->tableView_contents->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_contents->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_contents->setEditTriggers(QAbstractItemView::NoEditTriggers);    //禁止编辑
}

void MainWindow::on_actNoticeManage_triggered()
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    posterWork->setWorkType(1);
    emit posterWorking();

    //初始化Markdown解析
    ui->mContents_preview->setPage(notice_page);
    connect(ui->contents_editor, &QPlainTextEdit::textChanged,
        [this]() { m_content.setText(ui->contents_editor->toPlainText()); });
    notice_page->setWebChannel(m_channel);

    ui->mContents_preview->setUrl(QUrl("qrc:/images/index.html"));

    ui->tableView_mContents->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_mContents->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_mContents->setEditTriggers(QAbstractItemView::NoEditTriggers);    //禁止编辑
}

void MainWindow::on_actApplyList_triggered() 
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    emit loadApplyFormList(uid);
}

void MainWindow::on_actApplyItems_triggered() 
{
    if (ui->stackedWidget->currentIndex() == 13)
        return;
    ui->stackedWidget->setCurrentIndex(13);
    emit loadManagePageApplyItems(uid);
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

    ui->tableView_group->setRowHidden(0, true); //隐藏第一行
    ui->stackedWidget->setCurrentIndex(11);
    ui->stackedWidget->currentWidget()->setEnabled(true);
}

void MainWindow::on_actMore_triggered() 
{
    ui->stackedWidget->setCurrentIndex(12);
}

void MainWindow::on_actPanel_triggered()
{
    ui->stackedWidget->setCurrentIndex(13);
    emit loadStatisticsPanel();
}

void MainWindow::on_actRefresh_triggered()
{
    qDebug() << "心跳请求...";
    emit get_statistics();  //统计心跳请求量
    trayIcon->setToolTip("WePlanet - 运行中（上次刷新" + QDateTime::currentDateTime().time().toString("hh:mm") + "）");
    int index = ui->stackedWidget->currentIndex(); 
    switch (index)
    {
    case 0: on_actHome_triggered(); break;
    case 1: on_actMyInfo_triggered(); break;
    case 2: emit loadMsgMemList(uid); break;
    case 3: emit activityManageWorking(); break;
    case 4: emit attendWorking(); break;
    case 5: emit loadUserPageApplyItems(uid); break;
    case 6: on_actUserManager_triggered(); break;
    case 7: on_actAttendManager_triggered(); break;
    case 8: emit activityManageWorking(); break;
    case 9: emit loadApplyFormList(uid); break;
    case 10: loadManagePageApplyItems(uid); break;
    case 11: on_actGroup_triggered(); break;
    case 14: emit posterWorking(); break;
    case 15: emit posterWorking(); break;
    case 16: on_actPanel_triggered(); break;
    case 17: on_actSettings_triggered(); break;

    default:
        break;
    }
}

void MainWindow::on_actSettings_triggered()
{
    if (ui->groupBox_system->isEnabled())
    {
        emit loadSystemSettings();
        ui->label_loadingSettings->setMovie(loadingMovie);
    }
    ui->stackedWidget->setCurrentIndex(17);
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
    
    //ui->tableView_userManage->setItemDelegateForRow(current.row(), readOnlyDelegate);   //禁止编辑系统账号
    ui->tableView_userManage->setRowHidden(current.row(), curRecord.value("uid") == "1");
    if (preRecord.value("uid") == "1" && curRecord.value("uid") != "1")
		ui->tableView_userManage->setRowHidden(previous.row(), false);  //为选中系统账号时取消隐藏

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
        ui->label_userStatus->setText("状态：正常");
    else
        ui->label_userStatus->setText("状态：封禁");

    //子线程加载头像
    ui->userManagePage_avatar->setPixmap(QPixmap(":/images/color_icon/user.svg"));
    
    if (getAvatarQueue.isEmpty())
    {
        userManageWork->setCurAvatarUrl(curRecord.value("user_avatar").toString());
        emit userManageGetAvatar();
    }
    getAvatarQueue.enqueue(curRecord.value("user_avatar").toString());   //加载项入栈
    
	//子线程获取认证信息
    ui->label_verifyType_manage->setText("加载中...");
    ui->btn_verifyInfo->setEnabled(false);
    ui->btn_delVerify->setEnabled(false);
    
    if (getVerifyQueue.isEmpty())
        emit getVerify(curRecord.value("uid").toString());
    getVerifyQueue.enqueue(curRecord.value("uid").toString());   //加载项入栈
    
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
    QSqlRecord curRecord = attendUserModel->record(current.row());
    QSqlRecord curAttendRecord;
    //curDateTime = QDateTime::currentDateTime();

    ui->btn_attendManage_reAttend->setEnabled(current.isValid());
    ui->btn_attendManage_cancelAttend->setEnabled(current.isValid());
    QString uid = curRecord.value("uid").toString();
    //attendManageModel->setFilter("a_uid='" + curRecord.value("uid").toString() +"'");
    //curAttendRecord = attendManageModel->record(0);     //取最新考勤记录
    ui->label_attendManagePage_uid->setText(uid);
    
    for (int i = 0; i < attendManageModel->rowCount(); i++)
    {
        if (attendManageModel->record(i).value("a_uid").toString() == uid)
        {
			if (attendManageModel->record(i).value("today").toString() == curDateTime.date().toString("yyyy-MM-dd"))
                curAttendRecord = attendManageModel->record(i); //取最新考勤记录
            ui->tableView_attendInfo->showRow(i);
        }
        else
            ui->tableView_attendInfo->hideRow(i);
    }

    //子线程加载头像
    if (getAvatarQueue.isEmpty())
    {
        attendManageWork->setCurAvatarUrl(curRecord.value("user_avatar").toString());
        emit attendManageGetAvatar();
    }
    getAvatarQueue.enqueue(curRecord.value("user_avatar").toString());   //加载项入栈

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
	ui->activityManage_avatar->setPixmap(QPixmap(":/images/color_icon/user.svg"));
    ui->label_actMemUid->setText("加载中...");
    ui->label_actMemName->setText("加载中...");
    ui->label_actMemGroup->setText("加载中...");
    ui->label_actMemDpt->setText("加载中...");
    ui->label_actTel->setText("加载中...");
    ui->label_actMail->setText("加载中...");
    emit queryAccount(activityMemModel->record(current.row()).value("actm_uid").toString());
}

void MainWindow::on_myActivityPagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    QSqlRecord curRecord = activityMemModel->record(current.row()), curActRec;
    QString active_id = curRecord.value("act_id").toString();
    //QString pre_filter = activityModel->filter();
    //activityModel->setFilter("act_id=" + curRecord.value("act_id").toString());
    if (!activityModel->filter().isEmpty())
        on_btn_actSearchClear_clicked();
    for (int i = 0; i < activityMemModel->rowCount(); i++)
    {
		if (activityMemModel->record(i).value("act_id").toString() == active_id)
		{
            curActRec = activityModel->record(i);
			break;
		}
    }
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

    //activityModel->setFilter(pre_filter);
}

void MainWindow::on_activityManagePagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    actEditMapper->setCurrentIndex(current.row());  //将映射移动到对应行
    QSqlRecord curRec = activityModel->record(current.row());
	active_id = curRec.value("act_id").toString();
    //activityMemModel->setFilter("act_id=" + curRec.value("act_id").toString());
    //报名成员列表
    int mem_sum = 0;
    for (int i = 0; i < activityMemModel->rowCount(); i++)
    {
        if(activityMemModel->record(i).value("act_id").toString() != active_id)
            ui->tableView_actMember->hideRow(i);
        else
        {
            mem_sum++;
            ui->tableView_actMember->showRow(i);
        }
    }
    ui->lcdNumber_actMem->display(mem_sum);
    ui->rBtn_actAll->setChecked(true);
}

void MainWindow::on_comboBox_activity_currentIndexChanged(const QString& arg1)
{
    //curDateTime = QDateTime::currentDateTime();
    QString dateTime = curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    if (arg1 == "所有活动")
        activityModel->setFilter("");
    else
        activityModel->setFilter("joinDate <= '" + dateTime + "' AND beginDate >= '" + dateTime + "'");
}

void MainWindow::on_comboBox_myAct_currentIndexChanged(const QString& arg1)
{
    //curDateTime = QDateTime::currentDateTime();
    QString dateTime = curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    if (arg1 == "所有活动")
        activityMemModel->setFilter("actm_uid=" + uid);
    else if (arg1 == "未录取")
        activityMemModel->setFilter("actm_uid=" + uid + " AND status='未录取'");
    else if (arg1 == "待审核")
        activityMemModel->setFilter("actm_uid=" + uid + " AND status='待审核'");
    else
        activityMemModel->setFilter("actm_uid=" + uid + " AND (status='已录取' OR status='已完成')");
}

void MainWindow::on_noticeManagePagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    QSqlRecord curRecord = noticeManageModel->record(current.row());
    noticeEditMapper->setCurrentIndex(current.row());  //将映射移动到对应行
    ui->label_mContentCreated->setText(curRecord.value("created").toDateTime().toString("yyyy-MM-dd hh:mm"));
    ui->label_mContentModified->setText(curRecord.value("modified").toDateTime().toString("yyyy-MM-dd hh:mm"));
    if (curRecord.value("c_type").toInt() == 0)
        ui->rBtn_mCNotice->setChecked(true);
}

void MainWindow::on_myNoticePagecurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);
    QSqlRecord curRecord = noticeModel->record(current.row());
    ui->label_cTitle->setText(curRecord.value("title").toString());
    ui->label_contentAuthor->setText(curRecord.value("author_id").toString());
    ui->label_contentCreated->setText(curRecord.value("created").toDateTime().toString("yyyy-MM-dd hh:mm"));
    ui->label_contentModified->setText(curRecord.value("modified").toDateTime().toString("yyyy-MM-dd hh:mm"));
    if (curRecord.value("c_type").toInt() == 0)
        ui->label_contentType->setText("通知");
    else
        ui->label_contentType->setText("动态");
}

void MainWindow::on_btn_addGroup_clicked()
{

    groupModel->insertRow(groupModel->rowCount(),QModelIndex());    //在末尾添加一个记录
    QModelIndex curIndex = groupModel->index(groupModel->rowCount() - 1, 1);    //创建最后一行的ModelIndex
    groupPageSelection_group->clearSelection();//清空选择项
    groupPageSelection_group->setCurrentIndex(curIndex, QItemSelectionModel::Select);//设置刚插入的行为当前选择行

    //int currow = curIndex.row(); //获得当前行
    //groupModel->setData(groupModel->index(currow, 0), groupModel->rowCount()); //自动生成编号
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

void MainWindow::on_btn_actApproveAll_clicked()
{
    QSqlRecord record = activityModel->record(activitySelection->currentIndex().row());
    if (record.value("act_id").toString().isEmpty())
    {
        QMessageBox::warning(this, "警告", "请选择正确的活动。");
        return;
    }
    emit approveAllActivity(record.value("act_id").toString());
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
    if(!ui->checkBox_agreePrivacy->isChecked())
	{
		QMessageBox::warning(this, "警告", "请先阅读并同意《个人信息保护政策》。", QMessageBox::Ok);
		return;
	}

    //curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime()); //获取网络时间
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
    ui->checkBox_agreePrivacy->setChecked(false);
    emit applyActivity(select_id, uid);
}

void MainWindow::on_btn_actCancel_clicked()
{   
    //curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime()); //获取网络时间
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

    //int currow = curIndex.row(); //获得当前行
    //departmentModel->setData(departmentModel->index(currow, 0), departmentModel->rowCount()); //自动生成编号
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

void MainWindow::on_btn_getCnt_clicked()
{
    setStatisticsPanel(3, panel_series_count);
}

void MainWindow::on_rBtn_woman_clicked()
{
    ui->comboBox_group->setCurrentIndex(0);
    ui->comboBox_department->setCurrentIndex(0);
    userManageModel->setFilter("gender='女'");
}

void MainWindow::on_btn_registerCnt_clicked()
{
    setStatisticsPanel(2, panel_series_count);
}

void MainWindow::on_rBtn_all_clicked()
{
    ui->comboBox_group->setCurrentIndex(0);
    ui->comboBox_department->setCurrentIndex(0);
    userManageModel->setFilter("");
}

void MainWindow::on_btn_actCnt_clicked()
{
    setStatisticsPanel(4, panel_series_count);
}

void MainWindow::on_rBtn_actAll_clicked()
{
 //   int idx = -1;
	//QString pre_filter = activityMemModel->filter();
 //   idx = pre_filter.indexOf("AND");
 //   if (idx != -1)
 //       pre_filter = pre_filter.mid(0, idx-1);
 //   if (idx == -1 && pre_filter.indexOf("status") != -1)
 //       pre_filter.clear();
 //   activityMemModel->setFilter(pre_filter);
    for (int i = 0; i < activityMemModel->rowCount(); i++)
    {
        if (activityMemModel->record(i).value("act_id").toString() != active_id)
            ui->tableView_actMember->hideRow(i);
        else
			ui->tableView_actMember->showRow(i);
    }
}

void MainWindow::on_btn_dyCnt_clicked()
{
    setStatisticsPanel(5, panel_series_count);
}

void MainWindow::on_btn_saveSysSettings_clicked()
{
    setBaseInfoWork->setSys_isAnnounceOpen(ui->rBtn_announceOpen->isChecked());
    setBaseInfoWork->setSys_isTipsAnnounce(ui->rBtn_tipsAnnounce->isChecked());
    setBaseInfoWork->setSys_isDebugOpen(ui->rBtn_debugOpen->isChecked());
    setBaseInfoWork->setSys_openChat(ui->rBtn_openChat->isChecked());
    setBaseInfoWork->setSys_announcementText(ui->textEdit_announcement->toPlainText());
	
    ui->label_loadingSettings->setMovie(loadingMovie);
    emit saveSystemSettings();
}

void MainWindow::on_rBtn_actFinished_clicked()
{
  //  int idx = -1;
  //  QString pre_filter = activityMemModel->filter();
  //  idx = pre_filter.indexOf("AND");
  //  if (idx != -1)
  //      pre_filter = pre_filter.mid(0, idx-1);
  //  if (idx == -1 && pre_filter.indexOf("status") != -1)
  //      pre_filter.clear();
  //  if(pre_filter == "")
  //      activityMemModel->setFilter("status = '已录取' OR status = '未录取'");
  //  else
		//activityMemModel->setFilter(pre_filter + " AND (status='已录取' OR status='未录取')");
    for (int i = 0; i < activityMemModel->rowCount(); i++)
    {
		QString status = activityMemModel->record(i).value("status").toString();
		if (activityMemModel->record(i).value("act_id").toString() == active_id && (status == "已录取" || status == "未录取" || status == "已完成"))
            ui->tableView_actMember->showRow(i);
        else
            ui->tableView_actMember->hideRow(i);
    }
}

void MainWindow::on_rBtn_actPending_clicked()
{
    //int idx = -1;
    //QString pre_filter = activityMemModel->filter();
    //idx = pre_filter.indexOf("AND");
    //if (idx != -1)
    //    pre_filter = pre_filter.mid(0, idx-1);
    //if (idx == -1 && pre_filter.indexOf("status") != -1)
    //    pre_filter.clear();
    //if (pre_filter == "")
    //    activityMemModel->setFilter("status = '待审核'");
    //else
    //    activityMemModel->setFilter(pre_filter + " AND status = '待审核'");
    for (int i = 0; i < activityMemModel->rowCount(); i++)
    {
		if (activityMemModel->record(i).value("act_id").toString() == active_id && activityMemModel->record(i).value("status").toString() == "待审核")
            ui->tableView_actMember->showRow(i);
        else
            ui->tableView_actMember->hideRow(i);
    }
}

void MainWindow::on_btn_userManagePage_search_clicked()
{
    userManageModel->setFilter("uid='" + ui->lineEdit_searchUid->text()+ "' OR name='" + ui->lineEdit_searchUid->text() + "'");
}

void MainWindow::on_btn_userManagePage_recovery_clicked()
{
    userManageModel->setFilter("");
}

void MainWindow::on_btn_updateContent_clicked()
{
    QModelIndex curIndex;
    //curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime());
    bool res = noticeEditMapper->submit();
    if(res)
    {
        curIndex = noticeManageSelection->currentIndex();
        noticeManageModel->setData(noticeManageModel->index(curIndex.row(), noticeManageModel->fieldIndex("modified")), curDateTime);   //更新修改时间
        res = noticeManageModel->submitAll();
    }
    if (res)
    {
        QMessageBox::information(this, "消息", "通知·动态更新成功。", QMessageBox::Ok);
        on_actNoticeManage_triggered();
    }
    else
        QMessageBox::warning(this, "警告", "通知·动态更新失败，错误信息：" + noticeManageModel->lastError().text(), QMessageBox::Ok);
}

void MainWindow::on_btn_cancelContent_clicked()
{
    if (posterWork->cache)
    {
        noticeManageModel->revertAll();
        posterWork->cache = false;
        ui->btn_cancelContent->setText("放弃修改");
        ui->btn_delContent->setEnabled(true);
    }
    else
        noticeEditMapper->revert();
    ui->btn_addContent->setText("新增 通知·动态");
}

void MainWindow::on_btn_addContent_clicked()
{
    QModelIndex curIndex;
    if (!posterWork->cache)
    {
        QMessageBox::information(this, "消息", "已新增一条通知·动态，请在编辑完成后再次点击此按钮以完成发布。", QMessageBox::Ok);
        noticeManageModel->insertRow(noticeManageModel->rowCount(), QModelIndex()); //在末尾添加一个记录
        curIndex = noticeManageModel->index(noticeManageModel->rowCount() - 1, 1);//创建最后一行的ModelIndex
        posterWork->cacheRow = curIndex.row();
        noticeManageSelection->clearSelection();    //清空选择项
        noticeManageSelection->setCurrentIndex(curIndex, QItemSelectionModel::Select);//设置刚插入的行为当前选择行
    	posterWork->cache = true;
        ui->btn_delContent->setEnabled(false);
        ui->btn_addContent->setText("发布 通知·动态");
        ui->btn_cancelContent->setText("放弃发布");
        ui->rBtn_mCNotice->setChecked(true);
        ui->checkBox_mCisHide->setChecked(false);
    }else
    {
        if (ui->lineEdit_manageContents->text().isEmpty() || ui->contents_editor->toPlainText().isEmpty())
        {
            QMessageBox::warning(this, "警告", "请将标题、正文等编辑完成后再点击发布。", QMessageBox::Ok);
            return;
        }
        //curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime());
        noticeManageModel->setData(noticeManageModel->index(posterWork->cacheRow, noticeManageModel->fieldIndex("created")), curDateTime);
        noticeManageModel->setData(noticeManageModel->index(posterWork->cacheRow, noticeManageModel->fieldIndex("modified")), curDateTime);
        noticeManageModel->setData(noticeManageModel->index(posterWork->cacheRow, noticeManageModel->fieldIndex("author_id")), uid);

        const bool res = noticeEditMapper->submit();
        if (res)
        {
            posterWork->cache = false;
            ui->btn_addContent->setText("新增 通知·动态");
            ui->btn_cancelContent->setText("放弃修改");
            ui->btn_delContent->setEnabled(true);
            QMessageBox::information(this, "消息", "通知·动态发布成功。", QMessageBox::Ok);
            emit poster_statistics();   //统计活动发布量
            on_actNoticeManage_triggered();
        }
        else
            QMessageBox::warning(this, "警告", "通知·动态发布失败，错误信息：" + noticeManageModel->lastError().text(), QMessageBox::Ok);

    }
}

void MainWindow::on_btn_delContent_clicked()
{
    const QMessageBox::StandardButton res = QMessageBox::warning(this, "警告", "确认要删除该通知·动态吗？", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
	{
        const QModelIndex curIndex = noticeManageSelection->currentIndex();//获取当前选择单元格的模型索引
        noticeManageModel->removeRow(curIndex.row());
        noticeManageModel->submitAll();
        on_actNoticeManage_triggered();
    }
}

void MainWindow::on_lineEdit_manageContents_textChanged(const QString& arg1)
{
    ui->label_mCTitle->setText(arg1);
}

void MainWindow::on_btn_searchContents_clicked()
{
    QString arg = "(c_id='" + ui->lineEdit_searchContents->text() + "' OR title LIKE '%" + ui->lineEdit_searchContents->text() + "%' OR text LIKE '%" + ui->lineEdit_searchContents->text() + "%' OR author_id='" + ui->lineEdit_searchContents->text() + "') AND isHide=0";
    noticeModel->setFilter(arg);
}

void MainWindow::on_comboBox_group_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_group(1, ui->comboBox_group, ui->comboBox_department);
}

void MainWindow::on_btn_switchPanel_clicked()
{
	if(panel_series_count == 14)
        setStatisticsPanel(panel_option, 7);
    else
        setStatisticsPanel(panel_option, 14);
}

void MainWindow::on_btn_recoveryContents_clicked()
{
    noticeModel->setFilter("isHide=0");
    ui->lineEdit_searchContents->clear();
}

void MainWindow::on_btn_oneMonth_clicked()
{
    QMessageBox::StandardButton res;
    res = QMessageBox::warning(this, "警告", "确认仅保留所有用户一个月考勤数据吗？", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
        connect(loadingMovie, &QMovie::frameChanged, this, [=](int tmp)
            {
                Q_UNUSED(tmp);
                ui->btn_oneMonth->setIcon(QIcon(loadingMovie->currentPixmap()));
            });
        emit attendDataOperate(1);
    }
}

void MainWindow::on_btn_threeMonth_clicked()
{
    QMessageBox::StandardButton res;
    res = QMessageBox::warning(this, "警告", "确认仅保留所有用户三个月考勤数据吗？", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
        connect(loadingMovie, &QMovie::frameChanged, this, [=](int tmp)
            {
                Q_UNUSED(tmp);
                ui->btn_threeMonth->setIcon(QIcon(loadingMovie->currentPixmap()));
            });
        emit attendDataOperate(2);
    }
}

void MainWindow::on_btn_removeAll_clicked()
{
    QMessageBox::StandardButton res;
    res = QMessageBox::warning(this, "警告", "确认清除所有用户考勤数据吗？该操作不可逆，请谨慎操作。", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
        connect(loadingMovie, &QMovie::frameChanged, this, [=](int tmp)
            {
                Q_UNUSED(tmp);
                ui->btn_removeAll->setIcon(QIcon(loadingMovie->currentPixmap()));
            });
        emit attendDataOperate(3);
    }
}

void MainWindow::on_comboBox_department_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_dpt(1, ui->comboBox_group, ui->comboBox_department);
}

void MainWindow::on_btn_panel_clicked()
{
    setStatisticsPanel(0, panel_series_count);
}

void MainWindow::on_btn_userManagePage_recovery_2_clicked()
{
    userManageModel->setFilter("");
    ui->comboBox_group->setCurrentIndex(0);
    ui->comboBox_department->setCurrentIndex(0);
    ui->rBtn_all->setChecked(true);
}

void MainWindow::on_btn_loginCnt_clicked()
{
    setStatisticsPanel(1, panel_series_count);
}

void MainWindow::on_comboBox_group_2_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_group(2, ui->comboBox_group_2, ui->comboBox_department_2);
}

void MainWindow::on_comboBox_department_2_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    setUsersFilter_dpt(2, ui->comboBox_group_2, ui->comboBox_department_2);
}

void MainWindow::on_checkBox_agreePrivacy_stateChanged(int state)
{
	Q_UNUSED(state);
    //ui->btn_actJoin->setEnabled(bool(state));
}

void MainWindow::on_checkBox_autoRun_stateChanged(int state)
{
    setProcessAutoRun(QApplication::applicationFilePath(), state);
}

void MainWindow::on_checkBox_noMsgRem_stateChanged(int state)
{
    if(state)
        QMessageBox::information(this, "提示", "开启勿扰模式后，将关闭新消息及好友验证提醒。", QMessageBox::Ok);
}

void MainWindow::on_btn_resetAutoRun_clicked()
{
    setProcessAutoRun(QApplication::applicationFilePath(), 1);
    ui->checkBox_autoRun->setChecked(isAutoRun(QApplication::applicationFilePath()));
    QMessageBox::information(this, "提示", "设置完成，请重新启动Windows。", QMessageBox::Ok);
}

void MainWindow::on_btn_attendManagePage_recovery_clicked()
{
    attendUserModel->setFilter("");
    attendManageModel->setFilter("");
    ui->comboBox_group_2->setCurrentIndex(0);
    ui->comboBox_department_2->setCurrentIndex(0);
}

void MainWindow::on_btn_attendManagePage_search_clicked()
{
    attendUserModel->setFilter("uid='" + ui->lineEdit_searchUid_attend->text()+ "' OR name='" + ui->lineEdit_searchUid_attend->text() + "'");
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
    //curDateTime = QDateTime::currentDateTime();
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
    //curDateTime = QDateTime::currentDateTime();
    QSqlRecord curRecord;
    int delete_row;
    for (delete_row = 0; delete_row < attendManageModel->rowCount(); delete_row++)
    {
        if (!ui->tableView_attendInfo->isRowHidden(delete_row) && attendManageModel->record(delete_row).value("today").toString() == curDateTime.date().toString("yyyy-MM-dd"))
        {
            curRecord = attendManageModel->record(delete_row);
            break;
        }
    }
    if(ui->label_attendManagePage_status->text() == "未签到")
    {
        QMessageBox::warning(this, "消息", "当前用户未签到，无法退签。", QMessageBox::Ok);
        return;
    }
    //检测即将删除的数据是否与当前用户对应
    if(curRecord.value("a_uid").toString() == ui->label_attendManagePage_uid->text() && curRecord.value("today").toString() == curDateTime.date().toString("yyyy-MM-dd"))
    {
        attendManageModel->removeRow(delete_row); //删除顶部的数据
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
    //curDateTime = QDateTime::currentDateTime();
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
    //curDateTime = QDateTime::currentDateTime();
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
	//curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime()); //获取网络时间
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

void MainWindow::on_btn_switchChart_clicked()
{
    QString temp = eChartsJsCode;
    if (ui->label_chartMod->text() == "我的本周考勤数据")
    {
        eChartsJsCode = QString("init(%1, 2)").arg(eChartsJsCode);
        ui->label_chartMod->setText("本周总体考勤概况");
        ui->webEngineView_eCharts->page()->runJavaScript(eChartsJsCode);
        eChartsJsCode = temp;
    }
    else
    {
        eChartsJsCode = QString("init(%1, 1)").arg(eChartsJsCode);
        ui->label_chartMod->setText("我的本周考勤数据");
        ui->webEngineView_eCharts->page()->runJavaScript(eChartsJsCode);
        eChartsJsCode = temp;
    }
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
    connect(loadingMovie, &QMovie::frameChanged, this, [=](int tmp)
        {
            Q_UNUSED(tmp);
            ui->btn_personalSubmit->setIcon(QIcon(loadingMovie->currentPixmap()));
        });
    emit editPersonalInfo(ui->lineEdit_checkOldPwd->text(), newTel, newMail, newAvatar, newPwd);
}

void MainWindow::on_editPersonalInfoRes(int res)
{
    disconnect(loadingMovie, &QMovie::frameChanged, this, 0);
    ui->btn_personalSubmit->setIcon(QIcon());
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

void MainWindow::on_btn_verifyInfo_clicked()
{
    infoWidget->showMinimized();
    //QThread::msleep(150);
    infoWidget->showNormal();
}

void MainWindow::on_btn_delVerify_clicked()
{
    emit updateVerify(0, -1, "");
}

void MainWindow::on_btn_updateVerify_clicked()
{
    if (ui->lineEdit_verifyInfo->text().isEmpty())
    {
        QMessageBox::warning(this, "警告", "认证信息暂未填写，请填写后重试。", QMessageBox::Ok);
        return;
    }
    int updateType = 2;
    if (userManageWork->getVerifyTag() == -1)
        updateType = 1;
	if(ui->rBtn_verify_person->isChecked())
        emit updateVerify(updateType, 1, ui->lineEdit_verifyInfo->text());
    else
        emit updateVerify(updateType, 2, ui->lineEdit_verifyInfo->text());
    ui->lineEdit_verifyInfo->clear();
    ui->rBtn_verify_person->setChecked(true);
}

void MainWindow::on_btn_sendMsg_clicked()
{
    QString msgText = ui->textEdit_msg->toPlainText();
    if (msgText.isEmpty() || sendToUid == "-1")
        return;
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    //QDateTime curDateTime = QDateTime::fromSecsSinceEpoch(service::getWebTime());   //获取网络时间
    stream << uid << sendToUid << msgText << curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    ui->label_send->setMovie(loadingMovie);
    emit sendMessage(array);
    isSending = true;   //消息发送中...

    msg_contents += QString("<p align='right' style='margin-right:15px;color:#8d8d8d;font-size:10pt;'>%2 %3</p>").arg(ui->label_home_name->text(), curDateTime.toString("hh:mm:ss"));
    msg_contents += QString("<p align='right' style='margin-top:20px; margin-bottom:20px;margin-right:15px;font-size:12pt;'>%1 📨 </p>").arg(msgText);
    ui->textBrowser_msgHistory->clear();
    ui->textBrowser_msgHistory->insertHtml(QString("%1%2<p>").arg(msgHistoryInfo, msg_contents));
    
    //修复滚动条最大高度可能不正确的问题(未研究QT源码，暂不清楚误差产生原因...)
    int pageStep = ui->textBrowser_msgHistory->verticalScrollBar()->pageStep();
    int documentHeight = ui->textBrowser_msgHistory->document()->size().height();
    int scrollBarMax = ui->textBrowser_msgHistory->verticalScrollBar()->maximum();
    if (documentHeight - pageStep > scrollBarMax)
    {
        ui->textBrowser_msgHistory->verticalScrollBar()->setMaximum(documentHeight - pageStep);
        scrollBarMax = documentHeight - pageStep;
    }
	ui->textBrowser_msgHistory->verticalScrollBar()->setValue(scrollBarMax);    //滚动条移动至最大值
    ui->textEdit_msg->clear();
}

void MainWindow::on_btn_newMsgCheacked_clicked()
{
    ui->btn_newMsgCheacked->setEnabled(false);
    ui->label_newMsgIcon->setVisible(false);
    ui->label_newMsg->setVisible(false);
}

void MainWindow::on_btn_addMsgMem_clicked()
{
    friendsWidget->set_group_applyInfo_enabled(false);
    friendsWidget->set_group_addFriends_enabled(true);
    friendsWidget->initInfo();
    friendsWidget->showMinimized();
    //QThread::msleep(150);
    friendsWidget->showNormal();
}

void MainWindow::on_btn_deleteMsgMem_clicked()
{
    if (sendToUid == "-1")
    {
        QMessageBox::warning(this, "警告", "请选择一名好友后再试。", QMessageBox::Ok);
        return;
    }
    const QMessageBox::StandardButton res = QMessageBox::warning(this, "警告", "确认要删除好友 [" + sendToUid + "] 吗？", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
        ui->btn_deleteMsgMem->setEnabled(false);
        ui->btn_deleteMsgMem->setText("处理中...");
        emit delFriend(uid, sendToUid);
    }
}

void MainWindow::on_btn_friendInfo_clicked()
{
	if (sendToUid == "-1")
	{
		QMessageBox::warning(this, "错误", "请选择一名好友后再试。", QMessageBox::Ok);
		return;
	}
    friendInfoWidget->setTitle("好友资料");
    friendInfoWidget->hideButton(false);
    friendInfoWidget->showMinimized();
    friendInfoWidget->setUid(sendToUid);
    friendInfoWidget->showNormal();
}

void MainWindow::on_btn_shareMe_clicked()
{
    QClipboard* clipboard = QApplication::clipboard();  //获取系统剪切板指针
    QString text = QString("WePlanet 用户名片：\nUID：%1\n姓名：%2\n类别：%3\n部门：%4\n\n来源：WePlanet 用户分享").arg(ui->label_home_uid->text(), ui->label_home_name->text(), ui->label_home_group->text(), ui->label_home_department->text());
    clipboard->setText(text);
    QMessageBox::information(this, "WePlanet 用户分享", "已将我的名片分享至剪切板。", QMessageBox::Ok);
}

void MainWindow::on_btn_checkTime_clicked()
{
    qint32 webTimeSinceEpoch = service::getWebTime();
    QDateTime webTime = QDateTime::fromSecsSinceEpoch(webTimeSinceEpoch);
    QDateTime localTime = QDateTime::currentDateTime();
    ui->label_webTime->setText(webTime.toString("yyyy年MM月dd日 hh:mm:ss"));
    ui->label_localTime->setText(localTime.toString("yyyy年MM月dd日 hh:mm:ss"));

    if (webTimeSinceEpoch == -1)
    {
        ui->label_webTime->setText("--");
        QMessageBox::information(this, "时间校验", "网络时间获取失败，请检查网络连接。");
		return;
    }
    
    double marginMinutes = localTime.secsTo(webTime) / 60.0;    //计算时间差
    QString res;
    if (marginMinutes > 3 || marginMinutes < -3)
    {
        res = QString("校验完成，当前时间误差为：%1 分钟，不满足误差要求（3 min），考勤、活动等已禁用。").arg(marginMinutes);
        ui->btn_actJoin->setEnabled(false);
        ui->btn_actCancel->setEnabled(false);
        ui->btn_beginAttend->setEnabled(false);
        ui->btn_endAttend->setEnabled(false);
        ui->btn_sendMsg->setEnabled(false);
    }
    else
    {
        res = QString("校验完成，当前时间误差为：%1 分钟，满足误差要求（3 min），考勤、活动等已启用。").arg(marginMinutes);
        ui->btn_actJoin->setEnabled(true);
        ui->btn_actCancel->setEnabled(true);
        ui->btn_beginAttend->setEnabled(true);
        ui->btn_endAttend->setEnabled(true);
        ui->btn_sendMsg->setEnabled(true);
    }

    QMessageBox::information(this, "时间校验", res);
}

void MainWindow::on_btn_reManageApplyProcess_clicked()
{
    currentApplyItemAuditorList.clear();
    updateManageApplyItemProcess(QList<QString>());
    for (auto item : manageApplyAuditorList)
    {
        if (item != nullptr)
            item->setEnabled(true);
    }
}

void MainWindow::on_btn_reManageApplyOptions_clicked()
{
    ui->label_manageApplyOptions->clear();
    currentApplyItemOptions.clear();
}

void MainWindow::on_btn_manageApplyAddOption_clicked()
{
    //禁止空字段和$分割标识符
    if (ui->lineEdit_newApplyOption->text().isEmpty() || ui->lineEdit_newApplyOption->text().indexOf("$") != -1)
        return;
    currentApplyItemOptions.push_back(ui->lineEdit_newApplyOption->text());
    QString options;
    for (auto item : currentApplyItemOptions)
        options += QString("【%1】").arg(item);
    ui->label_manageApplyOptions->setText(options);
    ui->lineEdit_newApplyOption->clear();
}

void MainWindow::on_btn_manageApplyModify_clicked()
{
    if (currentApplyItemID_manage.isEmpty())
        return;
    ui->btn_manageApplyPublish->setEnabled(true);
    ui->groupBox_newApply->setEnabled(false);
    ui->btn_manageApplyPublish->setText("更新申请项目");
    isApplyItemEdit = true; //正在编辑
    ui->btn_manageApplyModify->setEnabled(false);
    ui->groupBox_addAuditors->setEnabled(true);
    ui->groupBox_addApplyOptions->setEnabled(true);
    
    ui->btn_manageApplyDelete->setEnabled(true);
    ui->btn_manageApplySwitch->setEnabled(true);
}

void MainWindow::on_lineEdit_msgPushTime_textChanged(const QString& arg)
{
    if (arg.toInt() > 0 && arg.toInt() <= 300)
    {
        msgPushTime = arg.toInt();
        config_ini->setValue("/System/MsgPushTime", msgPushTime);
        msgPushTimer->stop();
        msgPushTimer->start(msgPushTime * 1000);
    }
    else
        ui->lineEdit_msgPushTime->setText(QString::number(msgPushTime));
}

void MainWindow::on_lineEdit_msgPushMaxCnt_textChanged(const QString& arg)
{
    if (arg.toInt() > 0 && arg.toInt() <= 300)
    {
        msgStackMax = arg.toInt();
        config_ini->setValue("/System/MsgStackMaxCnt", msgStackMax);
    }
    else
        ui->lineEdit_msgPushMaxCnt->setText(QString::number(msgStackMax));
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
        connectStatusLable->setText("服务状态: " + sqlWork->getTestDb().lastError().text());
    }
    else
    {
        dbStatus = true;
        statusIcon->setPixmap(*statusOKIcon);
        connectStatusLable->setText("服务状态: 已连接至服务器");
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
        ui->groupBox_system->setEnabled(false);
        ui->groupBox_system->setVisible(false);
    }
    else {
        actionList[6]->setEnabled(true);
        actionList[6]->setVisible(true);
        ui->groupBox_system->setEnabled(true);
        ui->groupBox_system->setVisible(true);
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
    mShowMainAction->setIcon(QIcon(":/images/color_icon/color-computer.svg"));
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
	mExitAppAction->setIcon(QIcon(":/images/color_icon/color-delete.svg"));
    mShowExitAction = new QAction("切换账号", this);
	mShowExitAction->setIcon(QIcon(":/images/color_icon/color-reply.svg"));
    connect(mShowExitAction, &QAction::triggered, this, [=]()
        {
            on_actExit_triggered();
        });
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
