#include "formlogin.h"
#include "ui_formlogin.h"

void formLogin::send()
{
    emit sendData(db, ui->lineEdit_Uid->text());
}

formLogin::formLogin(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::formLogin)
{
    ui->setupUi(this);
    //限制账号只能输入数字
    QRegExp regx("[0-9]+$");
    QValidator* validator = new QRegExpValidator(regx, ui->lineEdit_Uid);
    ui->lineEdit_Uid->setValidator(validator);

    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮

    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
    service::connectDatabase(db);
    db.open();
    service::initDatabaseTables(db);
}

formLogin::~formLogin()
{
    delete ui;
}

void formLogin::on_btn_Login_clicked()
{

    if(service::authAccount(db, ui->lineEdit_Uid->text().toInt(), service::pwdEncrypt(ui->lineEdit_Pwd->text())))
        this->accept();
    else
        QMessageBox::warning(this, "登录失败", "用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
    db.close();
}
