#include "friendinfowidget.h"

FriendInfoWidget::FriendInfoWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::FriendInfoWidgetClass())
{
	ui->setupUi(this);
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

    //加载动画（PNG序列）
    aeMovieTimer = new QTimer(this);
    QDir aeDir("ae/loading");
    QFileInfoList listInfo = aeDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList m_strListImg;
    foreach(QFileInfo strFileInfo, listInfo) {
        m_strListImg.append(strFileInfo.filePath());
    }
    connect(aeMovieTimer, &QTimer::timeout, this, [=]() {
        static int cnt = 0;
        if (cnt >= m_strListImg.size())
            cnt = 0;
        if (Mailsending)
            ui->btn_sendMsg->setIcon(QPixmap(m_strListImg.at(cnt)));
        cnt++;
        });
    aeMovieTimer->start(20);

    clipboard = QApplication::clipboard();  //获取系统剪切板指针
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);   // 禁止改变窗口大小
    //子线程
    thread = new QThread();
    infoWork = new baseInfoWork();
    infoWork->moveToThread(thread);
    thread->start();
    
    connect(this, &FriendInfoWidget::getCurrentMemInfo, infoWork, &baseInfoWork::loadBaseInfoWorking);
    connect(infoWork, &baseInfoWork::baseInfoFinished, this, &FriendInfoWidget::setInfoPage);
}

void FriendInfoWidget::setLoading()
{
    ui->btn_share->setEnabled(false);
    ui->btn_sendMsg->setEnabled(false);
    ui->uid->setText("加载中...");
    ui->name->setText("加载中...");
    ui->department->setText("加载中...");
    ui->group->setText("加载中...");
    ui->phone->setText("加载中...");
    ui->mail->setText("加载中...");
    ui->verifyType->setText("加载中...");
    ui->verifyInfo->setText("--");
    ui->avatar->setPixmap(QPixmap(":/images/color_icon/user.svg"));
    ui->verifyIcon->setPixmap(QPixmap(":/images/color_icon/color-delete.svg"));
}

void FriendInfoWidget::setInfoPage()
{
    isLoading = false;
	ui->uid->setText(m_uid);
	ui->name->setText(infoWork->getName());
	ui->department->setText(infoWork->getDepartment());
	ui->group->setText(infoWork->getGroup());
	if(infoWork->getTel().isEmpty())
		ui->phone->setText("--");
    else
		ui->phone->setText(infoWork->getTel());
    if (infoWork->getMail().isEmpty())
    {
        m_mail.clear();
        ui->mail->setText("--");
    }
    else
    {
        ui->mail->setText(infoWork->getMail());
        m_mail = infoWork->getMail();
    }
	ui->verifyType->setText(infoWork->getVerifyType());
	ui->verifyInfo->setText(infoWork->getVerifyInfo());
    if(infoWork->getAvatar().isNull())
		ui->avatar->setPixmap(QPixmap(":/images/color_icon/user.svg"));
    else
		ui->avatar->setPixmap(infoWork->getAvatar());
    
    int verifyTag = infoWork->getVerifyTag();
    if (verifyTag == 1)
        ui->verifyIcon->setPixmap(QPixmap(":/images/color_icon/verify.svg"));
    else if (verifyTag == 2)
        ui->verifyIcon->setPixmap(QPixmap(":/images/color_icon/verify_2.svg"));

    if (verifyTag == -1)
    {
        ui->verifyIcon->setPixmap(QPixmap(":/images/color_icon/color-delete.svg"));
        ui->verifyType->setText("暂无认证");
        ui->verifyInfo->setText("--");
    }
    ui->btn_share->setEnabled(true);
    ui->btn_sendMsg->setEnabled(true);
}

void FriendInfoWidget::setUid(const QString& uid)
{
	m_uid = uid;
    infoWork->setUid(m_uid);
    setLoading();
    if (!isLoading)
    {
        emit getCurrentMemInfo();
        isLoading = true;
    }
}

void FriendInfoWidget::setTitle(const QString& title)
{
	this->setWindowTitle(title);
}

void FriendInfoWidget::setFromUserInfo(const QString& fromUserInfo)
{
    this->fromUserInfo = fromUserInfo;
}

void FriendInfoWidget::setSmtpConfig(const QList<QString> smtp_config)
{
    this->smtp_config = smtp_config;
}

void FriendInfoWidget::hideButton(bool isHide)
{
	bool isVisible = !isHide;
    ui->stackedWidget->setVisible(isVisible);
    this->adjustSize();
}

QString FriendInfoWidget::getMail()
{
    return m_mail;
}

void FriendInfoWidget::on_btn_share_clicked()
{
	QString text = QString("WePlanet 用户名片：\nUID：%1\n姓名：%2\n类别：%3\n部门：%4\n\n来源：WePlanet 用户分享").arg(m_uid, infoWork->getName(), infoWork->getGroup(), infoWork->getDepartment());
    clipboard->setText(text);
	QMessageBox::information(this, "WePlanet 用户分享", "已将用户资料分享至剪切板。", QMessageBox::Ok);
}

QString FriendInfoWidget::getUid()
{
	return m_uid;
}


FriendInfoWidget::~FriendInfoWidget()
{
    //在此处等待所有线程停止
    if (thread->isRunning())
    {
        thread->quit();
        thread->wait();
    }
    delete infoWork;
    delete thread;
	delete ui;
}

void FriendInfoWidget::on_btn_sendMsg_clicked()
{
    if (m_mail.isEmpty())
    {
        QMessageBox::warning(this, "警告", "该用户未设置邮箱，无法发送邮件提醒。", QMessageBox::Ok);
		return;
    }
    Mailsending = true;
    int smtp_rescode = service::sendMail(smtp_config, m_mail, QString("WePlanet 消息提醒"), QString("你的好友 %1 正在通过 [WePlanet 畅聊] 给你发送信息，请尽快前往 [WePlanet 畅聊] 查看/回复好友最新消息。").arg(fromUserInfo));

    if (smtp_rescode != 1)
        QMessageBox::warning(this, "警告", "邮件发送失败，请检查邮箱是否正确或联系管理员检查SMTP配置。", QMessageBox::Ok);
    else
		QMessageBox::information(this, "提示", "邮件提醒发送成功。", QMessageBox::Ok);
    Mailsending = false;
    ui->btn_sendMsg->setIcon(QIcon(":/images/color_icon/color-mail.svg"));
}

void FriendInfoWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        this->close(); break;
    default:
        QWidget::keyPressEvent(event);
    }
}