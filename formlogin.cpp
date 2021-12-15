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
    readPwd = readLoginSettings();
}

formLogin::~formLogin()
{
    delete ui;
}

void formLogin::writeLoginSettings()
{
    QSettings settings("bytecho", "magicgms");  //公司名称和应用名称
    if(ui->checkBox_remPwd)
    {
        settings.setValue("uid", ui->lineEdit_Uid->text());
        if(!(ui->lineEdit_Pwd->text() == "kH9bV0rP5dF8oW7g"))
            settings.setValue("pwd", service::pwdEncrypt(ui->lineEdit_Pwd->text()));
        settings.setValue("isSaveAccount", ui->checkBox_remPwd->isChecked());
    }
    else
        settings.setValue("isSaveAccount", ui->checkBox_remPwd->isChecked());
}

QString formLogin::readLoginSettings()
{
    QSettings settings("bytecho", "magicgms");  //公司名称和应用名称
    bool isSaveAccount = settings.value("isSaveAccount", false).toBool();
    ui->lineEdit_Uid->setText(settings.value("uid").toString());
    if(isSaveAccount)
    {
        ui->checkBox_remPwd->setChecked(true);
        ui->lineEdit_Pwd->setText("kH9bV0rP5dF8oW7g");  //填入伪密码，代表密码读取成功
        return settings.value("pwd").toString();
    }
    return "";
}

void formLogin::on_btn_Login_clicked()
{
    QString pwd = readPwd;
    QSettings settings("bytecho", "magicgms");  //公司名称和应用名称
    if(service::authAccount(db, ui->lineEdit_Uid->text().toInt(), pwd) || service::authAccount(db, ui->lineEdit_Uid->text().toInt(), service::pwdEncrypt(ui->lineEdit_Pwd->text())))
    {
        writeLoginSettings();   //验证成功后，保存账号密码
        this->accept();
    }
    else
        QMessageBox::warning(this, "登录失败", "用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
    db.close();
}

void formLogin::on_checkBox_remPwd_clicked(bool checked)
{
    if(!checked)
    {
        ui->lineEdit_Pwd->clear();
        readPwd.clear();
    }
    writeLoginSettings();
}


void formLogin::on_lineEdit_Uid_textEdited(const QString &arg1)
{
    QSettings settings("bytecho", "magicgms");  //公司名称和应用名称
    if(arg1 !=  settings.value("uid"))  //防止账户变化时还保存着之前的用户信息
    {
        settings.clear();
        readPwd.clear();
    }
}
