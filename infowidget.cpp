#include "infowidget.h"

InfoWidget::InfoWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

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

InfoWidget::~InfoWidget()
{
}

void InfoWidget::setInfo(const QString& info)
{
    ui.label_info->setText(info);
}

void InfoWidget::setInfoTitle(const QString& title)
{
    ui.label_infoType->setText(title);
}

void InfoWidget::setInfoIcon(const QPixmap& icon)
{
    ui.label_icon->setPixmap(icon);
}

void InfoWidget::setBoxTitle(const QString& title)
{
    ui.groupBox->setTitle(title);
}

void InfoWidget::on_btn_ok_clicked()
{
	this->close();
}