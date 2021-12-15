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

    connectStatusLable = new QLabel("Database Status: connecting...");
    connectStatusLable->setMinimumWidth(250);

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
    ui->label_uid->setText(uid);

    QPixmap statusOKIcon(":/images/color_icon/color-approve.svg"), statusErrorIcon(":/images/color_icon/color-delete.svg");

    if (db.open())   //打开数据库
    {
        statusIcon->setPixmap(statusOKIcon);
        connectStatusLable->setText("Database Status: connected");
    }
    else
    {
        statusIcon->setPixmap(statusErrorIcon);
        connectStatusLable->setText("Database Status: " + db.lastError().text());
    }
}


void MainWindow::on_actExit_triggered()
{
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
}

void MainWindow::on_actMore_triggered()
{
    ui->stackedWidget->setCurrentIndex(9);
}
