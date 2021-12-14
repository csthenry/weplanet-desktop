#include "formlogin.h"
#include "ui_formLogin.h"

formLogin::formLogin(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::formLogin)
{
    ui->setupUi(this);
    //限制账号只能输入数字
    QRegExp regx("[0-9]+$");
    QValidator* validator = new QRegExpValidator(regx, ui->lineEdit_Uid);
    ui->lineEdit_Uid->setValidator(validator);


}

formLogin::~formLogin()
{
    delete ui;
}

void formLogin::on_btn_Login_clicked()
{

    QSqlDatabase db;
    service::connectDatabase(db);
    db.open();
    if(service::authAccount(db, ui->lineEdit_Uid->text().toInt(), service::pwdEncrypt(ui->lineEdit_Pwd->text())))
        this->accept();
    else
        QMessageBox::warning(this, "登录失败", "用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
    db.close();
}
