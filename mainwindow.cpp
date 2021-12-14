#include "mainwindow.h"
#include "ui_mainwindow.h"

QSqlDatabase MainWindow::connectDatabase()
{

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("magic");
    db.setUserName("root");
    db.setPassword("123456");

    if (!db.open())   //打开数据库
        QMessageBox::warning(this, "错误", "打开数据库失败",
                                 QMessageBox::Ok,QMessageBox::NoButton);
    else

        QMessageBox::information(this, "提示", "打开数据库成功",
                                 QMessageBox::Ok,QMessageBox::NoButton);
    return db;

}

void MainWindow::initDatabaseTables(QSqlDatabase db)
{

        if(!db.open()){
            qDebug() << "error info :" << db.lastError();
        }
        else{
            QSqlQuery query;
            QString creatTableStr = "CREATE TABLE magic_users   \
                    (                                           \
                      uid           int(10)      NOT NULL AUTO_INCREMENT,     \
                      password      varchar(64)  NOT NULL ,         \
                      name          varchar(32)  NOT NULL ,     \
                      gender        char(1)      NULL ,         \
                      telephone     varchar(64)  NULL ,         \
                      mail          varchar(128) NULL ,         \
                      user_group         char(1)      NULL ,         \
                      user_position      char(1)      NULL ,         \
                      PRIMARY KEY (uid)           \
                    )ENGINE=InnoDB;";

            query.prepare(creatTableStr);
            if(!query.exec()){
                qDebug()<<"query error :"<<query.lastError();
            }
            else{
                qDebug()<<"creat table success!";
            }
        }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initDatabaseTables(connectDatabase());
}

MainWindow::~MainWindow()
{
    delete ui;
}

