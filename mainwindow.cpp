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

    connectStatusLable = new QLabel("Database Connection status: connecting...");
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
        connectStatusLable->setText("Database Connection status: connected");
    }
    else
    {
        statusIcon->setPixmap(statusErrorIcon);
        connectStatusLable->setText("Database status: " + db.lastError().text());
    }
}


void MainWindow::on_actExit_triggered()
{
    formLoginWindow = new formLogin();
    this->hide();
    connect(formLoginWindow, SIGNAL(sendData(QSqlDatabase, QString)), this, SLOT(receiveData(QSqlDatabase, QString)));    //接收登录窗口的信号
    if (formLoginWindow->exec() == QDialog::Accepted)
    {
        formLoginWindow->send();    //发送信号
        delete formLoginWindow;
        this->show();
    }
    else
        delete formLoginWindow;

}
