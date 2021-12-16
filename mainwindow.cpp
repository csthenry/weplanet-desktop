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
        ui->label_home_tel->setText(query.value(2).toString());
        ui->label_home_mail->setText(query.value(3).toString());
        ui->avatar->setPixmap(service::setAvatarStyle(service::getAvatar(query.value(6).toString())));
        ui->label_verifyIcon->setPixmap(*verifyIcon);
        ui->label_home_group->setText(service::getGroup(uid));
        ui->label_home_department->setText(service::getDepartment(uid));

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
    if(!db.open())
    {
        statusIcon->setPixmap(*statusErrorIcon);
        connectStatusLable->setText("Database Status: " + db.lastError().text());
    }
    else
    {
        queryModel tableModel(db, this);
        ui->tableView_department->setModel(tableModel.setActGroupPage_departmentModel());
        ui->tableView_group->setModel(tableModel.setActGroupPage_groupModel());
    }
}

void MainWindow::on_actMore_triggered()
{
    ui->stackedWidget->setCurrentIndex(9);
}
