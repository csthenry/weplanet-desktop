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
    ui->label_verifyIcon->setScaledContents(true);
    connectStatusLable = new QLabel("Database Status: connecting...");
    connectStatusLable->setMinimumWidth(250);
    statusOKIcon = new QPixmap(":/images/color_icon/color-approve.svg"), statusErrorIcon = new QPixmap(":/images/color_icon/color-delete.svg");

    verifyIcon = new QPixmap(":/images/color_icon/verify_2.svg");

    ui->statusbar->addWidget(statusIcon);   //将状态组件添加至statusBar
    ui->statusbar->addWidget(connectStatusLable);
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
        ui->label_home_name->setText(query.value(0).toString());
        ui->label_home_gender->setText(query.value(1).toString());
        if(ui->label_home_gender->text().isEmpty()) ui->label_home_gender->setText("--");   //将可能为空的数据设置值
        ui->label_home_tel->setText(query.value(2).toString());
        if(ui->label_home_tel->text().isEmpty()) ui->label_home_tel->setText("--");
        ui->label_home_mail->setText(query.value(3).toString());
        if(ui->label_home_mail->text().isEmpty()) ui->label_home_mail->setText("--");
        ui->avatar->setPixmap(service::setAvatarStyle(service::getAvatar(query.value(6).toString())));
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

void MainWindow::on_actExit_triggered()
{
    QSettings settings("bytecho", "magicgms");
    settings.setValue("isAutoLogin", false);    //注销后自动登录失效

    formLoginWindow = new formLogin();
    this->close();
    connect(formLoginWindow, SIGNAL(sendData(QSqlDatabase, QString)), this, SLOT(receiveData(QSqlDatabase, QString)));    //接收登录窗口的信号
    if (formLoginWindow->exec() == QDialog::Accepted)
    {
        formLoginWindow->send();    //发送信号
        //reload函数，用于重载窗口数据
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
}

void MainWindow::on_actAttendManager_triggered()
{
    ui->stackedWidget->setCurrentIndex(5);
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
    QStringList comboxList;

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
        connect(groupPageSelection_group, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_groupPageGroupcurrentChanged(QModelIndex,QModelIndex)));
        //选择行变化时
        connect(groupPageSelection_department, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                    this, SLOT(on_groupPageDptcurrentChanged(QModelIndex,QModelIndex)));

        ui->tableView_group->setSelectionModel(groupPageSelection_group);
        ui->tableView_department->setSelectionModel(groupPageSelection_department);

        ui->tableView_group->setRowHidden(0, true);
        ui->tableView_group->setRowHidden(1, true);

        //权限设置combox
        comboxList << "1" << "0";
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

void MainWindow::on_btn_addGroup_clicked()
{

    groupModel->insertRow(groupModel->rowCount(),QModelIndex()); //在末尾添加一个记录
    QModelIndex curIndex = groupModel->index(groupModel->rowCount() - 1, 1);//创建最后一行的ModelIndex
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
    departmentModel->insertRow(departmentModel->rowCount(),QModelIndex()); //在末尾添加一个记录
    QModelIndex curIndex = departmentModel->index(departmentModel->rowCount() - 1, 1);//创建最后一行的ModelIndex
    groupPageSelection_department->clearSelection();//清空选择项
    groupPageSelection_department->setCurrentIndex(curIndex, QItemSelectionModel::Select);//设置刚插入的行为当前选择行

    int currow = curIndex.row(); //获得当前行
    departmentModel->setData(departmentModel->index(currow, 0), departmentModel->rowCount()); //自动生成编号
}

void MainWindow::on_btn_delDpt_clicked()
{
    QModelIndex curIndex = groupPageSelection_department->currentIndex();//获取当前选择单元格的模型索引
    departmentModel->removeRow(curIndex.row()); //删除最后一行
}

void MainWindow::on_btn_delGroup_clicked()
{
    QModelIndex curIndex = groupPageSelection_group->currentIndex();//获取当前选择单元格的模型索引
    groupModel->removeRow(curIndex.row()); //删除最后一行
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
