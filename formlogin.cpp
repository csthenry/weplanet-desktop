#include "formlogin.h"
#include "ui_formlogin.h"

void formLogin::send()
{
    emit sendData(db, loginUid);
}

formLogin::formLogin(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::formLogin)
{
    ui->setupUi(this);

    //限制登录注册输入
    QRegExp regx_account("[0-9]{1,11}$"), regx_pwd("[0-9A-Za-z!@#$%^&*.?]{1,16}$");
    QValidator* validator_account = new QRegExpValidator(regx_account), *validator_pwd= new QRegExpValidator(regx_pwd);
    ui->lineEdit_Uid->setValidator(validator_account);
    ui->lineEdit_SignupTel->setValidator(validator_account);
    ui->lineEdit_Pwd->setValidator(validator_pwd);
    ui->lineEdit_SignupPwd->setValidator(validator_pwd);
    ui->lineEdit_SignupPwdAgain->setValidator(validator_pwd);

    ui->labelStatus->setWhatsThis("如果显示绿色图标，表示数据库连接正常，否则请检查网络或联系技术支持。\nEmail: cst@bytecho.net");

    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮

    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
    QPixmap mainicon(":/images/color_icon/main.svg"), statusOKIcon(":/images/color_icon/color-approve.svg"), statusErrorIcon(":/images/color_icon/color-delete.svg");
    ui->labelIcon->setMaximumSize(25, 25);
    ui->labelIcon->setScaledContents(true);    //图片自适应大小
    ui->mainIcon->setScaledContents(true);
    ui->mainIcon->setPixmap(mainicon);

    service loginService;
    loginService.connectDatabase(db);

    db.open();

    if (db.open())   //打开数据库
    {
        ui->labelIcon->setPixmap(statusOKIcon);
        ui->labelStatus->setText("Database Status: connected");
    }
    else
    {
        ui->labelIcon->setPixmap(statusErrorIcon);
        ui->labelStatus->setText("Database Status: " + db.lastError().text());
    }
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
        if(ui->checkBox_autoLogin->isChecked())
            settings.setValue("isAutoLogin", ui->checkBox_remPwd->isChecked());
    }
    else
    {
        settings.setValue("isSaveAccount", ui->checkBox_remPwd->isChecked());
        settings.setValue("isAutoLogin", false);    //如果不记住密码，那么自动登录失效
    }
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
        if(settings.value("isAutoLogin", false).toBool())
        {
            ui->checkBox_autoLogin->setChecked(true);
            if(service::authAccount(db, loginUid, ui->lineEdit_Uid->text().toLongLong(), settings.value("pwd").toString()))     //自动登录
            {
                writeLoginSettings();   //验证成功后，保存账号密码
                autoLoginSuccess = true;
            }
            else
            {
                autoLoginSuccess = false;
                QMessageBox::warning(this, "登录失败", "用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
            }
        }
        return settings.value("pwd").toString();
    }
    return "";
}

void formLogin::on_btn_Login_clicked()
{
    QString pwd = readPwd;
    QSettings settings("bytecho", "magicgms");  //公司名称和应用名称

    if(service::authAccount(db, loginUid, ui->lineEdit_Uid->text().toLongLong(), pwd) || service::authAccount(db, loginUid, ui->lineEdit_Uid->text().toLongLong(), service::pwdEncrypt(ui->lineEdit_Pwd->text())))
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

void formLogin::on_btn_Signup_clicked()
{
    QSqlQuery query;
    QString creatTableStr;
    if(ui->lineEdit_SignupName->text().isEmpty() || ui->lineEdit_SignupTel->text().isEmpty() || ui->lineEdit_SignupPwd->text().isEmpty() || ui->lineEdit_SignupPwdAgain->text().isEmpty())
    {
        QMessageBox::warning(this, "警告", "注册失败，请检查是否已经填写全所有注册所需信息。", QMessageBox::Ok);
        return;
    }
    if(ui->lineEdit_SignupPwd->text() != ui->lineEdit_SignupPwdAgain->text())
    {
        QMessageBox::warning(this, "警告", "注册失败，请检查密码填写是否一致。", QMessageBox::Ok);
        return;
    }
    if(ui->lineEdit_SignupPwd->text().length() < 6)
    {
        QMessageBox::warning(this, "警告", "注册失败，请输入6~16位的密码以确保安全。", QMessageBox::Ok);
        return;
    }
    creatTableStr =
            "INSERT INTO magic_users"
            "(password, name, user_group, telephone )"
            "VALUES                        "
            "(:pwd, :name, 2, :phone) ";
    query.prepare(creatTableStr);
    query.bindValue(0, service::pwdEncrypt(ui->lineEdit_SignupPwd->text()));
    query.bindValue(1, ui->lineEdit_SignupName->text());
    query.bindValue(2, ui->lineEdit_SignupTel->text());
    if(query.exec())
    {
        QMessageBox::information(this, "通知", "注册成功，你的信息如下：\n账号：" + query.lastInsertId().toString()+"\n姓名：" + ui->lineEdit_SignupName->text() +"\n手机号：" +ui->lineEdit_SignupTel->text() + "\n请妥善保管以上信息，可使用手机号登录。", QMessageBox::Ok);
    }
    else
        QMessageBox::warning(this, "警告", "注册失败，错误信息：" + db.lastError().text() + "\n" + query.lastError().text(), QMessageBox::Ok);
}

void formLogin::on_lineEdit_Pwd_textEdited(const QString &arg1)
{
    QSettings settings("bytecho", "magicgms");  //公司名称和应用名称
    if(arg1 != "kH9bV0rP5dF8oW7g")  //防止账户变化时还保存着之前的用户信息
    {
        settings.clear();
        readPwd.clear();
    }
}
