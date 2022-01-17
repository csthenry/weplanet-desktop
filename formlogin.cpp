/***************************************************/
/*              Magic Light Assistant              */
/* Copyright (c) 2017-2021 by bytecho.net          */
/* Written by Henry                                */
/* Function:                                       */
/* Communication, activity, management and approval*/
/***************************************************/

#include "formlogin.h"
#include "ui_formlogin.h"

void formLogin::send()
{
    emit sendData(loginUid);
}

formLogin::formLogin(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::formLogin)
{
    ui->setupUi(this);
    this->setEnabled(false);
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
    QPixmap mainicon(":/images/color_icon/main.svg");
    statusOKIcon = new QPixmap(":/images/color_icon/color-approve.svg"), statusErrorIcon = new QPixmap(":/images/color_icon/color-delete.svg");
    ui->labelIcon->setMaximumSize(25, 25);
    ui->labelIcon->setScaledContents(true);    //图片自适应大小
    ui->mainIcon->setScaledContents(true);
    ui->mainIcon->setPixmap(mainicon);

    //多线程相关
    sqlWork = new SqlWork("loginDB");    //sql异步连接
    loginWork = new baseInfoWork();
    sqlThread = new QThread(), dbThread = new QThread();

    sqlWork->moveToThread(dbThread);
    loginWork->moveToThread(sqlThread);

    //开启数据库连接线程
    dbThread->start();
    sqlWork->beginThread();
    connect(this, &formLogin::startDbWork, sqlWork, &SqlWork::working);
    emit startDbWork();
    connect(sqlWork, SIGNAL(newStatus(bool)), this, SLOT(on_statusChanged(bool)));    //数据库心跳验证 5s

    sqlThread->start();

    //登录相关
    connect(this, SIGNAL(authAccount(const long long, const QString&, const QString&)), loginWork, SLOT(authAccount(const long long, const QString&, const QString&)));
    connect(this, SIGNAL(autoLoginAuthAccount(const long long, const QString&)), loginWork, SLOT(autoAuthAccount(const long long, const QString&)));
    connect(loginWork, SIGNAL(authRes(bool)), this, SLOT(on_authAccountRes(bool)));
    connect(loginWork, SIGNAL(autoAuthRes(bool)), this, SLOT(on_autoLoginAuthAccountRes(bool)));
    //注册相关
    connect(this, SIGNAL(signUp(const QString&, const QString&, const QString&)), loginWork, SLOT(signUp(const QString&, const QString&, const QString&)));
    connect(loginWork, SIGNAL(signupRes(bool)), SLOT(on_signUpFinished(bool)));
    //初始化相关
    connect(sqlWork, &SqlWork::firstFinished, this, [=](){
        sqlWork->stopThread();
        this->setEnabled(true);
        readPwd = readLoginSettings();
        loginWork->setDB(sqlWork->getDb());
    });

}

formLogin::~formLogin()
{
    //在此处等待所有线程停止

    delete ui;
    delete loginWork;
    delete sqlWork;
    delete dbThread;
    delete sqlThread;
}

void formLogin::writeLoginSettings()
{
    QSettings settings("bytecho", "MagicLightAssistant");  //公司名称和应用名称
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

void formLogin::beforeAccept()
{
    if(sqlThread->isRunning())
    {
        sqlThread->quit();
        sqlThread->wait();
    }
    if(dbThread->isRunning())
    {
        sqlWork->stopThread();
        sqlWork->quit();
        dbThread->quit();
        dbThread->wait();
    }
}

QString formLogin::readLoginSettings()
{
    QSettings settings("bytecho", "MagicLightAssistant");  //公司名称和应用名称
    bool isSaveAccount = settings.value("isSaveAccount", false).toBool();
    ui->lineEdit_Uid->setText(settings.value("uid").toString());
    if(isSaveAccount)
    {
        ui->checkBox_remPwd->setChecked(true);
        ui->lineEdit_Pwd->setText("kH9bV0rP5dF8oW7g");  //填入伪密码，代表密码读取成功
        if(settings.value("isAutoLogin", false).toBool())
        {
            ui->checkBox_autoLogin->setChecked(true);
            emit autoLoginAuthAccount(ui->lineEdit_Uid->text().toLongLong(), settings.value("pwd").toString());
        }
        return settings.value("pwd").toString();
    }
    return "";
}

bool formLogin::autoLogin()
{
    return autoLoginSuccess;
}

void formLogin::on_btn_Login_clicked()
{
    QString pwd = readPwd;
    QSettings settings("bytecho", "MagicLightAssistant");  //公司名称和应用名称
    sqlWork->stopThread();
    emit authAccount(ui->lineEdit_Uid->text().toLongLong(), pwd, service::pwdEncrypt(ui->lineEdit_Pwd->text()));
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
    QSettings settings("bytecho", "MagicLightAssistant");  //公司名称和应用名称
    if(arg1 !=  settings.value("uid"))  //防止账户变化时还保存着之前的用户信息
    {
        settings.clear();
        readPwd.clear();
    }
}

void formLogin::on_btn_Signup_clicked()
{
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
    emit signUp(ui->lineEdit_SignupPwd->text(), ui->lineEdit_SignupName->text(), ui->lineEdit_SignupTel->text());

}

void formLogin::on_signUpFinished(bool res)
{
    if(res)
    {
        QMessageBox::information(this, "通知", "注册成功，你的信息如下：\n账号：" + loginWork->getLastSignupUid() + "\n姓名：" + ui->lineEdit_SignupName->text() +"\n手机号：" + ui->lineEdit_SignupTel->text() + "\n请妥善保管以上信息，可使用手机号登录。", QMessageBox::Ok);
    }
    else
        QMessageBox::warning(this, "警告", "注册失败，错误信息：" + sqlWork->getDb().lastError().text(), QMessageBox::Ok);
}

void formLogin::on_lineEdit_Pwd_textEdited(const QString &arg1)
{
    QSettings settings("bytecho", "MagicLightAssistant");  //公司名称和应用名称
    if(arg1 != "kH9bV0rP5dF8oW7g")  //防止账户变化时还保存着之前的用户信息
    {
        settings.clear();
        readPwd.clear();
    }
}

void formLogin::on_statusChanged(const bool status)
{
    if(status)
    {
        ui->labelIcon->setPixmap(*statusOKIcon);
        ui->labelStatus->setText("Database Status: connected");
    }
    else
    {
        ui->labelIcon->setPixmap(*statusErrorIcon);
        ui->labelStatus->setText("Database Status: " + sqlWork->getDb().lastError().text());
    }
}

void formLogin::on_authAccountRes(bool res)
{
    if(res)
    {
        loginUid = loginWork->getLoginUid();
        writeLoginSettings();   //验证成功后，保存账号密码
        beforeAccept();
        this->accept();
    }
    else
        QMessageBox::warning(this, "登录失败", "用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
    sqlWork->beginThread();
}

void formLogin::on_autoLoginAuthAccountRes(bool res)
{
    if(res)     //自动登录
    {
        loginUid = loginWork->getLoginUid();
        writeLoginSettings();   //验证成功后，保存账号密码
        autoLoginSuccess = true;
        beforeAccept();
        this->accept();
    }
    else
    {
        autoLoginSuccess = false;
        ui->checkBox_autoLogin->setCheckable(false);
        QMessageBox::warning(this, "登录失败", "用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
        sqlWork->beginThread();
    }
    qDebug() << "自动登录验证完成";
}
