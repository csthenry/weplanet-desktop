#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSqlDatabase db;
    service::connectDatabase(db);

    QPixmap statusOKIcon(":/images/color_icon/color-approve.svg"), statusErrorIcon(":/images/color_icon/color-delete.svg");
    statusIcon = new QLabel();  //用于显示状态图标的lable
    statusIcon->setMaximumSize(25, 25);
    statusIcon->setScaledContents(true);    //图片自适应大小
    statusIcon->setPixmap(statusErrorIcon);

    connectStatusLable = new QLabel("Database Connection status: connecting...");
    connectStatusLable->setMinimumWidth(250);

    ui->statusbar->addWidget(statusIcon);   //将状态组件添加至statusBar
    ui->statusbar->addWidget(connectStatusLable);

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

MainWindow::~MainWindow()
{
    delete ui;
}

