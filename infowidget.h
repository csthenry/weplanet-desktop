#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QFontDatabase>
#include <QKeyEvent>
#include "ui_infowidget.h"

class InfoWidget : public QWidget
{
	Q_OBJECT

public:
	InfoWidget(QWidget *parent = nullptr);
	~InfoWidget();
	void setInfo(const QString& info);
	void setInfoTitle(const QString& title);
	void setInfoIcon(const QPixmap& icon);
	void setBoxTitle(const QString& title);

protected:
	void keyPressEvent(QKeyEvent* event);	//ÊÂ¼þ¹ýÂËÆ÷
	
private:
	Ui::InfoWidgetClass ui;

private slots:
	void on_btn_ok_clicked();
};
