#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QLabel>
#include "ui_ovowidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OvOWidgetClass; };
QT_END_NAMESPACE

class OvOWidget : public QWidget
{
	Q_OBJECT

public:
	OvOWidget(QWidget *parent = nullptr);
	~OvOWidget();
private:
	QItemSelectionModel* selectionModel;
	QString ovoMatrix[8][14];	//用于存储表情矩阵
private slots:
	void ovoUpdate(const QModelIndex& current, const QModelIndex& previous);
	void OvOClicked(int row, int column);
private:
	Ui::OvOWidgetClass *ui;

signals:
	void OvOChanged(QString ovo);
};
