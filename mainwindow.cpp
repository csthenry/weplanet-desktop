#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, QDialog *formLoginWindow)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    statusIcon = new QLabel();  //用于显示状态图标的lable
    statusIcon->setMaximumSize(25, 25);

    statusIcon->setScaledContents(true);    //图片自适应大小
    ui->avatar->setScaledContents(true);
    ui->userManagePage_avatar->setScaledContents(true);
    ui->label_verifyIcon->setScaledContents(true);
    ui->attendManagePage_avatar->setScaledContents(true);

    connectStatusLable = new QLabel("Database Status: connecting...");
    connectStatusLable->setMinimumWidth(250);
    statusOKIcon = new QPixmap(":/images/color_icon/color-approve.svg"), statusErrorIcon = new QPixmap(":/images/color_icon/color-delete.svg");

    verifyIcon = new QPixmap(":/images/color_icon/verify_2.svg");

    ui->statusbar->addWidget(statusIcon);   //将状态组件添加至statusBar
    ui->statusbar->addWidget(connectStatusLable);
    ui->stackedWidget->setCurrentIndex(0);  //转到首页
    connect(formLoginWindow, SIGNAL(sendData(QSqlDatabase, QString)), this, SLOT(receiveData(QSqlDatabase, QString)));    //接收登录窗口的信号
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::receiveData(QSqlDatabase db, QString uid)
{
    this->db = db;
    this->uid = uid;
    ui->label_home_uid->setText(uid);

    if(this->db.open())   //打开数据库
    {
        statusIcon->setPixmap(*statusOKIcon);
        connectStatusLable->setText("Database Status: connected");
    }
    else
    {
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("Database Status: " + this->db.lastError().text());
    }
    this->db.close();
    setHomePageBaseInfo();
}

void MainWindow::setHomePageBaseInfo()
{
    if(!db.open())
    {
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("Database Status: " + db.lastError().text());
    }
    QSqlQuery query;
    query.exec("SELECT name, gender, telephone, mail, user_group, user_dpt, user_avatar FROM magic_users WHERE uid = " + uid);
    if(query.next())
    {
        ui->label_home_name->setText(query.value("name").toString());
        ui->label_home_gender->setText(query.value("gender").toString());
        if(ui->label_home_gender->text().isEmpty()) ui->label_home_gender->setText("--");   //将可能为空的数据设置值
        ui->label_home_tel->setText(query.value("telephone").toString());
        if(ui->label_home_tel->text().isEmpty()) ui->label_home_tel->setText("--");
        ui->label_home_mail->setText(query.value("mail").toString());
        if(ui->label_home_mail->text().isEmpty()) ui->label_home_mail->setText("--");
        ui->avatar->setPixmap(service::setAvatarStyle(service::getAvatar(query.value("user_avatar").toString())));
        ui->label_home_group->setText(service::getGroup(uid));
        ui->label_home_department->setText(service::getDepartment(uid));

        ui->label_verifyIcon->setPixmap(*verifyIcon);

    }
    else
    {
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("Database Status: " + query.lastError().text());
    }
    db.close();
}

void MainWindow::setUsersTypeCombox(QComboBox *group, QComboBox *department)
{
    //初始化数据过滤模块combox
    QSqlQuery comboxGroup;
    QStringList comboxItems;
    comboxGroup.exec("SELECT * FROM magic_group");
    while(comboxGroup.next())
        comboxItems << comboxGroup.value("group_name").toString();
    group->clear();
    group->addItem("所有用户组");
    group->addItems(comboxItems);
    comboxItems.clear();
    comboxGroup.exec("SELECT * FROM magic_department");
    while(comboxGroup.next())
        comboxItems << comboxGroup.value("dpt_name").toString();
    department->clear();
    department->addItem("所有部门");
    department->addItems(comboxItems);
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

void MainWindow::setUsersFilter_dpt(QComboBox *group, QComboBox *department)
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


void MainWindow::on_actExit_triggered()
{
    QSettings settings("bytecho", "MagicLightAssistant");
    settings.setValue("isAutoLogin", false);    //注销后自动登录失效

    formLoginWindow = new formLogin();
    this->close();
    connect(formLoginWindow, SIGNAL(sendData(QSqlDatabase, QString)), this, SLOT(receiveData(QSqlDatabase, QString)));    //接收登录窗口的信号
    if (formLoginWindow->exec() == QDialog::Accepted)
    {
        formLoginWindow->send();    //发送信号
        //reload函数，用于重载窗口数据
        ui->stackedWidget->setCurrentIndex(0);  //回到首页
        this->show();
    }
    delete formLoginWindow;

}

void MainWindow::on_actHome_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_actMyInfo_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_actAttend_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_actApply_triggered()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_actUserManager_triggered()
{
    ui->stackedWidget->setCurrentIndex(4);
    ui->tableView_userManage->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_userManage->setSelectionMode(QAbstractItemView::SingleSelection);
    if(!db.open())
    {
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("Database Status: " + db.lastError().text());
    }
    else
    {
        queryModel relTableModel(db, this);
        userManageModel = relTableModel.setActUserPage_relationalTableModel();
        ui->tableView_userManage->setModel(userManageModel);
        ui->tableView_userManage->hideColumn(userManageModel->fieldIndex("password"));  //隐藏密码列

        userManagePageSelection = new QItemSelectionModel(userManageModel);
        //当前项变化时触发currentChanged信号
        connect(userManagePageSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                    this, SLOT(on_userManagePagecurrentChanged(QModelIndex, QModelIndex)));

        //当前行变化时触发currentRowChanged信号
        connect(userManagePageSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                    this, SLOT(on_userManagePagecurrentRowChanged(QModelIndex, QModelIndex)));

        ui->tableView_userManage->setSelectionModel(userManagePageSelection);
        //设置ItemDelegate
        comboxList.clear();
        comboxList << "男" << "女";
        comboxDelegateGender.setItems(comboxList, false);
        ui->tableView_userManage->setItemDelegateForColumn(userManageModel->fieldIndex("gender"), &comboxDelegateGender);
        ui->tableView_userManage->setItemDelegate(new QSqlRelationalDelegate(ui->tableView_userManage));

        //初始化数据过滤模块combox
        setUsersTypeCombox(ui->comboBox_group, ui->comboBox_department);

    }

}

void MainWindow::on_actAttendManager_triggered()
{
    ui->stackedWidget->setCurrentIndex(5);
    ui->tableView_attendUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_attendUsers->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tableView_attendInfo->setSelectionBehavior(QAbstractItemView::SelectRows);

    if(!db.open())
    {
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("Database Status: " + db.lastError().text());
    }
    else
    {
        queryModel relTableModel(db, this), relTableModel_attend(db, this);

        //用户列表
        userManageModel = relTableModel.setActUserPage_relationalTableModel();
        ui->tableView_attendUsers->setModel(userManageModel);
        ui->tableView_attendUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);  //不可编辑
        ui->tableView_attendUsers->hideColumn(userManageModel->fieldIndex("password"));  //隐藏无关列
        ui->tableView_attendUsers->hideColumn(userManageModel->fieldIndex("user_avatar"));
        ui->tableView_attendUsers->hideColumn(userManageModel->fieldIndex("gender"));

        userManagePageSelection = new QItemSelectionModel(userManageModel);     //套用用户管理页的selection
        ui->tableView_attendUsers->setSelectionModel(userManagePageSelection);

        //当前行变化时触发currentRowChanged信号
        connect(userManagePageSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                    this, SLOT(on_attendManagePageUserscurrentRowChanged(QModelIndex, QModelIndex)));
        //初始化数据过滤comBox
        setUsersTypeCombox(ui->comboBox_group_2, ui->comboBox_department_2);

        //签到列表
        attendManageModel = relTableModel_attend.setActAttendPage_relationalTableModel();
        ui->tableView_attendInfo->setModel(attendManageModel);
        ui->tableView_attendInfo->setItemDelegate(new QSqlRelationalDelegate(ui->tableView_attendInfo));
        ui->tableView_attendInfo->hideColumn(attendManageModel->fieldIndex("num"));     //隐藏不需要的签到编号

    }
}

void MainWindow::on_actApplyList_triggered()
{
    ui->stackedWidget->setCurrentIndex(6);
}

void MainWindow::on_actApplyItems_triggered()
{
    ui->stackedWidget->setCurrentIndex(7);
}

void MainWindow::on_actGroup_triggered()
{
    ui->stackedWidget->setCurrentIndex(8);
    bool isEditable = false;

    //tableView显示属性设置
    ui->tableView_group->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_group->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_group->setAlternatingRowColors(true);
    ui->tableView_department->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_department->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_department->setAlternatingRowColors(true);
    //ui->tableView->resizeColumnsToContents();
    //ui->tableView->horizontalHeader()->setStretchLastSection(true);

    if(!db.open())
    {
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("Database Status: " + db.lastError().text());
    }
    else
    {
        queryModel tableModel(db, this);
        groupModel = tableModel.setActGroupPage_groupModel(), departmentModel = tableModel.setActGroupPage_departmentModel();

        ui->tableView_group->setModel(groupModel);
        ui->tableView_department->setModel(departmentModel);

        groupPageSelection_group = new QItemSelectionModel(groupModel);
        groupPageSelection_department = new QItemSelectionModel(departmentModel);

        //当前项变化时触发currentChanged信号
        connect(groupPageSelection_group, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                    this, SLOT(on_groupPageGroupcurrentChanged(QModelIndex, QModelIndex)));
        //选择行变化时
        connect(groupPageSelection_department, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                    this, SLOT(on_groupPageDptcurrentChanged(QModelIndex, QModelIndex)));

        ui->tableView_group->setSelectionModel(groupPageSelection_group);
        ui->tableView_department->setSelectionModel(groupPageSelection_department);

        ui->tableView_group->setRowHidden(0, true);
        ui->tableView_group->setRowHidden(1, true);

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
    }

}

void MainWindow::on_actMore_triggered()
{
    ui->stackedWidget->setCurrentIndex(9);
}

void MainWindow::on_groupPageDptcurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    Q_UNUSED(current);
    ui->btn_editDpt_cancel->setEnabled(departmentModel->isDirty());
    ui->btn_editDpt_check->setEnabled(departmentModel->isDirty());

    ui->btn_delDpt->setEnabled(current.isValid());
}

void MainWindow::on_groupPageGroupcurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    Q_UNUSED(current);
    ui->btn_editGroup_cancel->setEnabled(groupModel->isDirty());
    ui->btn_editGroup_check->setEnabled(groupModel->isDirty());

    ui->btn_delGroup->setEnabled(current.isValid());
}

void MainWindow::on_userManagePagecurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    QSqlRecord curRecord = userManageModel->record(current.row());
    ui->btn_editUser_check->setEnabled(userManageModel->isDirty());
    ui->btn_editUser_cancel->setEnabled(userManageModel->isDirty());

    if(curRecord.value("uid") != "100000" && curRecord.value("uid") != "1"  && curRecord.value("uid") != uid)  //避免删除初始用户和当前用户
        ui->btn_delUser->setEnabled(current.isValid());
    else
        ui->btn_delUser->setEnabled(false);

}

void MainWindow::on_userManagePagecurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QSqlRecord curRecord = userManageModel->record(current.row()), preRecord = userManageModel->record(previous.row());
    QPixmap avatar = service::getAvatar(curRecord.value("user_avatar").toString());

    ui->btn_editUser_check->setEnabled(userManageModel->isDirty());
    ui->btn_editUser_cancel->setEnabled(userManageModel->isDirty());

    if(curRecord.value("uid") != "100000" && curRecord.value("uid") != "1" && curRecord.value("uid") != uid)  //避免删除初始用户和当前用户
        ui->btn_delUser->setEnabled(current.isValid());
    else
        ui->btn_delUser->setEnabled(false);

    ui->label_userManagePage_uid->setText(curRecord.value("uid").toString());
    if(curRecord.value("name").toString().isEmpty())
        ui->label_userManagePage_name->setText("--");
    else
        ui->label_userManagePage_name->setText(curRecord.value("name").toString());
    ui->label_userManagePage_group->setText(curRecord.value("group_name").toString());    //这里应该填写关联的外键字段名
    ui->label_userManagePage_dpt->setText(curRecord.value("dpt_name").toString());

    if(avatar.isNull())
        ui->userManagePage_avatar->setText("无头像");
    else
        ui->userManagePage_avatar->setPixmap(service::setAvatarStyle(avatar));
    //密码修改
    if(!ui->lineEdit_editPwd->text().isEmpty())
    {
        if(ui->lineEdit_editPwd->text() == ui->lineEdit_editPwdCheck->text())
        {
            userManageModel->setData(userManageModel->index(previous.row(), userManageModel->fieldIndex("password")), service::pwdEncrypt(ui->lineEdit_editPwd->text()), Qt::EditRole);
            QMessageBox::information(this, "提示", "当前用户（UID：" + preRecord.value("uid").toString()+ "）密码已成功缓存。\n点击确认修改即可生效（请确认UID是否正确*）。\n新密码为：" + ui->lineEdit_editPwd->text(), QMessageBox::Ok);
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
    QSqlQuery attendQuery;
    QPixmap avatar = service::getAvatar(curRecord.value("user_avatar").toString());
    curDateTime = QDateTime::currentDateTime();

    ui->btn_attendManage_reAttend->setEnabled(current.isValid());
    ui->btn_attendManage_cancelAttend->setEnabled(current.isValid());

    ui->label_attendManagePage_uid->setText(curRecord.value("uid").toString());

    attendQuery.exec("SELECT * FROM magic_attendance WHERE a_uid = '" + curRecord.value("uid").toString() + "' AND today = '" + curDateTime.date().toString("yyyy-MM-dd") + "';");
    //qDebug() <<"SELECT * FROM magic_attendance WHERE uid = '" + curRecord.value("uid").toString() + "' AND today = '" + curDateTime.date().toString("yyyy-MM-dd") + "';";
    if(attendQuery.next())
    {
        ui->label_attendManagePage_status->setText("已签到");
        ui->label_attendManagePage_beginTime->setText(attendQuery.value("begin_date").toString());
        if(attendQuery.value("end_date").toString().isEmpty())
            ui->label_attendManagePage_endTime->setText("--");
        else
            ui->label_attendManagePage_endTime->setText(attendQuery.value("end_date").toString());
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

    if(avatar.isNull())
        ui->attendManagePage_avatar->setText("无头像");
    else
        ui->attendManagePage_avatar->setPixmap(service::setAvatarStyle(avatar));

    attendManageModel->setFilter("a_uid='" + curRecord.value("uid").toString() +"'");


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

void MainWindow::on_btn_editGroup_check_clicked()
{
    bool res = groupModel->submitAll();

    if (!res)
        QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + groupModel->lastError().text(),
                                 QMessageBox::Ok);
    else
    {
        ui->btn_editGroup_check->setEnabled(false);
        ui->btn_editGroup_cancel->setEnabled(false);
    }
}

void MainWindow::on_btn_editGroup_cancel_clicked()
{
    groupModel->revertAll();
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
    departmentModel->removeRow(curIndex.row()); //删除
}

void MainWindow::on_btn_delGroup_clicked()
{
    QModelIndex curIndex = groupPageSelection_group->currentIndex();//获取当前选择单元格的模型索引
    groupModel->removeRow(curIndex.row()); //删除
}

void MainWindow::on_btn_editDpt_check_clicked()
{
    bool res = departmentModel->submitAll();

    if (!res)
        QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + departmentModel->lastError().text(),
                                 QMessageBox::Ok);
    else
    {
        ui->btn_editDpt_check->setEnabled(false);
        ui->btn_editDpt_cancel->setEnabled(false);
    }
}

void MainWindow::on_btn_editDpt_cancel_clicked()
{
    departmentModel->revertAll();
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
}

void MainWindow::on_btn_delUser_clicked()
{
    QModelIndex curIndex = userManagePageSelection->currentIndex();//获取当前选择单元格的模型索引
    userManageModel->removeRow(curIndex.row()); //删除
}

void MainWindow::on_btn_editUser_check_clicked()
{
    bool res = userManageModel->submitAll();
    if(!res)
        QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + userManageModel->lastError().text(),
                                 QMessageBox::Ok);
    else{
        ui->btn_editUser_check->setEnabled(false);
        ui->btn_editUser_cancel->setEnabled(false);
    }
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
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("isSupply")), 1);
    attendManageModel->setData(attendManageModel->index(currow, attendManageModel->fieldIndex("name")), uid);   //这里要填外键关联的字段！
    if(attendManageModel->submitAll())
        QMessageBox::information(this, "消息", "补签成功，补签时间:\n" + curDateTime.toString("yyyy-MM-dd hh:mm:ss"),
                                 QMessageBox::Ok);
    else
        QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + attendManageModel->lastError().text(),
                                 QMessageBox::Ok);
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
        if(attendManageModel->submitAll())
            QMessageBox::information(this, "消息", "当前用户已被成功退签，当日签到数据已经删除。", QMessageBox::Ok);
        else
            QMessageBox::warning(this, "消息", "保存数据失败，错误信息:\n" + attendManageModel->lastError().text(),
                                     QMessageBox::Ok);
    }
    else
        QMessageBox::warning(this, "消息", "数据不匹配，退签失败。", QMessageBox::Ok);

}
