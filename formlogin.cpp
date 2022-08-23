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
    //QPixmap mainicon(":/images/color_icon/main.svg");
    //QPixmap mainicon(":/images/logo/planet.svg");
    statusOKIcon = new QPixmap(":/images/color_icon/color-approve.svg"), statusErrorIcon = new QPixmap(":/images/color_icon/color-delete.svg");
    ui->labelIcon->setMaximumSize(25, 25);
    ui->labelIcon->setScaledContents(true);    //图片自适应大小
    ui->mainIcon->setScaledContents(true);
    //ui->mainIcon->setPixmap(mainicon);
    //ui->labelIcon->setPixmap(QPixmap(":/images/color_icon/color-setting_2.svg"));

    //加载动画
    loadingMovie = new QMovie(":/images/color_icon/loading.gif");
    loadingMovie_2 = new QMovie(":/images/color_icon/loading.gif");
    connect(loadingMovie_2, &QMovie::frameChanged, this, [=](int tmp)
        {
            Q_UNUSED(tmp);
            ui->labelIcon->setPixmap(loadingMovie_2->currentPixmap());
        });
    connect(loadingMovie, &QMovie::frameChanged, this, [=](int tmp)
        {
            Q_UNUSED(tmp);
            ui->btn_Login->setIcon(QIcon(loadingMovie->currentPixmap()));
			ui->btn_Signup->setIcon(QIcon(loadingMovie->currentPixmap()));
        });
    loadingMovie_2->start();
    ui->labelIcon->setMovie(loadingMovie_2);

    //多线程相关
    sqlWork = new SqlWork("loginDB");    //sql异步连接
    loginWork = new baseInfoWork();
    sqlThread = new QThread(), dbThread = new QThread();
    updateSoftWare = new checkUpdate();

    sqlWork->moveToThread(dbThread);
    loginWork->moveToThread(sqlThread);
    updateSoftWare->moveToThread(sqlThread);

    //开启数据库连接线程
    dbThread->start();
    sqlWork->beginThread();
    connect(this, &formLogin::startDbWork, sqlWork, &SqlWork::working);
    emit startDbWork();
    connect(sqlWork, SIGNAL(newStatus(bool)), this, SLOT(on_statusChanged(bool)));    //数据库心跳验证 5s
    sqlThread->start();

    //config.ini 初始化数据库
    config_ini = new QSettings("config.ini", QSettings::IniFormat);
    connect(this, &formLogin::initDatabase, loginWork, &baseInfoWork::initDatabaseTables);
    connect(loginWork, &baseInfoWork::initDatabaseFinished, this, [=](bool res)
        {
            if (res)
                config_ini->setValue("/Database/init", true);
            else
                config_ini->setValue("/Database/init", false);
        }, Qt::UniqueConnection);

    //登录相关
    connect(this, SIGNAL(authAccount(const long long, const QString&, const QString&)), loginWork, SLOT(authAccount(const long long, const QString&, const QString&)));
    connect(this, SIGNAL(autoLoginAuthAccount(const long long, const QString&)), loginWork, SLOT(autoAuthAccount(const long long, const QString&)));
    connect(loginWork, SIGNAL(authRes(int)), this, SLOT(on_authAccountRes(int)));
    connect(loginWork, SIGNAL(autoAuthRes(int)), this, SLOT(on_autoLoginAuthAccountRes(int)));
    //注册相关
    connect(this, SIGNAL(signUp(const QString&, const QString&, const QString&, const QString&)), loginWork, SLOT(signUp(const QString&, const QString&, const QString&, const QString&)));
    connect(loginWork, SIGNAL(signupRes(bool)), SLOT(on_signUpFinished(bool)));
    //初始化相关
    readPwd = readLoginSettings();  //载入保存的账号信息
    connect(this, &formLogin::beginUpdate, updateSoftWare, &checkUpdate::homeCheckUpdate);
    //更新检测
    emit beginUpdate();
    connect(updateSoftWare, &checkUpdate::homeCheckUpdateFinished, this, &formLogin::updateFinished);
    connect(sqlWork, &SqlWork::firstFinished, this, [=](){
        if (!readPwd.isEmpty() && isAutoLogin)
        {
            loadingMovie->start();
            emit autoLoginAuthAccount(ui->lineEdit_Uid->text().toLongLong(), readPwd);
        }
        if (!config_ini->value("/Database/init").toBool())
        {
            emit initDatabase();    //初始化数据库
        }
    }, Qt::UniqueConnection);

    //更新HarmonyOS字体
    QFont font;
    int font_Id = QFontDatabase::addApplicationFont(":/src/font/HarmonyOS_Sans_SC_Regular.ttf");
    QStringList fontName = QFontDatabase::applicationFontFamilies(font_Id);
    font.setFamily(fontName.at(0));
    auto listWidget = findChildren<QWidget*>();
    for (auto& widget : listWidget) //遍历所有组件
    {
        font.setWeight(widget->font().weight());
        font.setPointSize(widget->font().pointSize());
        widget->setFont(font);
    }
}

formLogin::~formLogin()
{
    //在此处等待所有线程停止
    if (!isQuit)
        beforeAccept();
    loadingMovie->stop();
    loadingMovie_2->stop();

    delete loadingMovie;
    delete loadingMovie_2;
    delete ui;
    delete updateSoftWare;
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
    isQuit = true;
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
        isAutoLogin = settings.value("isAutoLogin", false).toBool();
        if(isAutoLogin)
            ui->checkBox_autoLogin->setChecked(true);

        return settings.value("pwd").toString();
    }
    return "";
}

void formLogin::updateFinished(bool res)
{
    if(!res)
        QMessageBox::critical(this, "检查失败", "服务器地址错误或JSON格式错误！\n错误信息：" + updateSoftWare->getErrorInfo());
    else if(updateSoftWare->getLatestVersion() > updateSoftWare->getCurVersion())
    {
        QString str = updateSoftWare->getUpdateString();
        int ret = QMessageBox::warning(this, "检查更新", str, "前往下载", "暂不更新");
        if (ret == 0)
        {
            QDesktopServices::openUrl(updateSoftWare->getUrl());
            this->close();  //关闭窗口
        }
    }
}

bool formLogin::autoLogin()
{
    return autoLoginSuccess;
}

void formLogin::on_btn_Login_clicked()
{
    QString pwd = readPwd;
    QInputDialog input(this);
    input.setFont(this->font());
    input.setWindowTitle("请输入验证码");
    input.setLabelText("你尝试登录的失败次数过多，请输入验证码：");
    input.setTextEchoMode(QLineEdit::Password);

    sqlWork->stopThread();
    if(loginErrCnt >= 3)
    {
        curTime = QTime::currentTime();
        qsrand(curTime.msec() + curTime.second() * 1000);
        QString tmpKey = QString::number(qrand() % 8999 + 1000);    //产生随机验证码(1000~9999)
        input.setLabelText("你尝试登录的失败次数过多，请输入验证码：" + tmpKey);
        input.setTextValue("");
        bool res = input.exec();
        QString text = input.textValue();
        while(!res || text != tmpKey)
        {
            curTime = QTime::currentTime();
            qsrand(curTime.msec() + curTime.second() * 1000);
            tmpKey = QString::number(qrand() % 8999 + 1000);    //产生随机验证码(1000~9999)
            input.setLabelText("验证码错误，请重新输入验证码：" + tmpKey);
            input.setTextValue("");
            res = input.exec();
            text = input.textValue();
        }
        loginErrCnt--;
        loadingMovie->start();
        emit authAccount(ui->lineEdit_Uid->text().toLongLong(), pwd, service::pwdEncrypt(ui->lineEdit_Pwd->text()));
        return;
    }
    loadingMovie->start();
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
    if(ui->lineEdit_SignupName->text().isEmpty() || ui->lineEdit_SignupTel->text().isEmpty() || ui->lineEdit_SignupPwd->text().isEmpty() || ui->lineEdit_SignupPwdAgain->text().isEmpty() || ui->comBox_gender->currentIndex() == 0)
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
    loadingMovie->start();
    emit signUp(ui->lineEdit_SignupPwd->text(), ui->lineEdit_SignupName->text(), ui->lineEdit_SignupTel->text(), ui->comBox_gender->currentText());
}

void formLogin::on_signUpFinished(bool res)
{
    loadingMovie->stop();
    ui->btn_Signup->setIcon(QIcon());
    ui->btn_Login->setIcon(QIcon());
    if(res)
    {
        QMessageBox::information(this, "通知", "注册成功，你的信息如下：\n账号：" + loginWork->getLastSignupUid() + "\n姓名：" + ui->lineEdit_SignupName->text() +"\n手机号：" + ui->lineEdit_SignupTel->text() + "\n请妥善保管以上信息，可使用手机号登录。", QMessageBox::Ok);
        ui->lineEdit_SignupName->clear();
        ui->lineEdit_SignupTel->clear();
        ui->lineEdit_SignupPwd->clear();
        ui->lineEdit_SignupPwdAgain->clear();
        ui->comBox_gender->setCurrentIndex(0);
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
    loadingMovie_2->stop();
    ui->btn_Login->setEnabled(status);
    ui->btn_Signup->setEnabled(status);
    if(status)
    {
        ui->labelIcon->setPixmap(*statusOKIcon);
        ui->labelStatus->setText("数据库服务状态: 已连接数据库");
    }
    else
    {
        ui->labelIcon->setPixmap(*statusErrorIcon);
        ui->labelStatus->setText("数据库服务状态: " + sqlWork->getTestDb().lastError().text());
    }
}

void formLogin::on_authAccountRes(int res)
{
    if(res == 200)
    {
        loginUid = loginWork->getLoginUid();
        writeLoginSettings();   //验证成功后，保存账号密码
        this->hide();
        beforeAccept();
        this->accept();
    }
    else
    {
        loadingMovie->stop();
        ui->btn_Signup->setIcon(QIcon());
        ui->btn_Login->setIcon(QIcon());
        if (res == 403)
        {
            QMessageBox::warning(this, "登录失败", "错误代码：403\n用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
            loginErrCnt++;  //登录失败的次数，大于3次需要输入验证码
        } else if(res == 500)
            QMessageBox::warning(this, "登录失败", "错误代码：500\n连接服务器失败，请检查网络连接或联系管理员。", QMessageBox::Yes);
        else if(res == 400)
			QMessageBox::warning(this, "登录失败", "错误代码：400\n由于你违反用户协议，你的账号已被封禁，请联系管理员。", QMessageBox::Yes);
        sqlWork->beginThread();
    }
    qDebug() << "登录验证状态码：" << res;
}

void formLogin::on_autoLoginAuthAccountRes(int res)
{
    if(res == 200)     //自动登录
    {
        loginUid = loginWork->getLoginUid();
        writeLoginSettings();   //验证成功后，保存账号密码
        autoLoginSuccess = true;
        this->hide();
        beforeAccept();
        this->accept();
    }
    else
    {
        autoLoginSuccess = false;
        ui->checkBox_autoLogin->setCheckable(false);
        loadingMovie->stop();
        ui->btn_Signup->setIcon(QIcon());
        ui->btn_Login->setIcon(QIcon());
        if (res == 403)
        {
            QMessageBox::warning(this, "登录失败", "错误代码：403\n用户验证失败，请检查用户名（UID）和密码。", QMessageBox::Yes);
            loginErrCnt++;  //登录失败的次数，大于3次需要输入验证码
        }
        else if (res == 500)
            QMessageBox::warning(this, "登录失败", "错误代码：500\n连接服务器失败，请检查网络连接或联系管理员。", QMessageBox::Yes);
        else if (res == 400)
            QMessageBox::warning(this, "登录失败", "错误代码：400\n由于你违反用户协议，你的账号已被封禁，请联系管理员。", QMessageBox::Yes);
        sqlWork->beginThread();
    }
    qDebug() << "自动登录验证完成";
}
