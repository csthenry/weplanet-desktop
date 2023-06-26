#include "ovowidget.h"
#include <QDebug>

OvOWidget::OvOWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::OvOWidgetClass())
{
	ui->setupUi(this);
	// https://doc.qt.io/qt-5/qtwidgets-widgets-windowflags-example.html
	setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);	//Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint |  Qt::MSWindowsFixedSizeDialogHint
	setAttribute(Qt::WA_TranslucentBackground);

	selectionModel = new QItemSelectionModel(ui->tableWidget->model());
	ui->tableWidget->setSelectionModel(selectionModel);
	ui->tableWidget->setFocusPolicy(Qt::NoFocus);
	QObject::connect(selectionModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(ovoUpdate(QModelIndex, QModelIndex)));
	QObject::connect(ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(OvOClicked(int, int)));
	QString ovo = "😀,😁,😂,🤣,😃,😄,😅,😆,😉,😊,😋,😎,😍,😘,🥰,😗,😙,🥲,😚,🙂,🤗,🤩,🤔,🫡,🤨,😐,😑,😶,🫥,😶,‍🌫️,😏,😣,😥,😮,🤐,😯,😪,😫,🥱,😴,😌,😛,😜,😝,🤤,😒,😓,😔,😕,🫤,🙃,🫠,🤑,😲,🙁,😖,😞,😟,😤,😢,😭,😦,😧,😨,😩,🤯,😬,😮‍💨,😰,😱,🥵,🥶,😳,🤪,😵,😵‍💫,🥴,😠,😡,🤬,😷,🤒,🤕,🤢,🤮,🤧,😇,🥳,🥸,🥺,🥹,🤠,🤡,🤥,🤫,🤭,🫢,🫣,🧐,🤓,😈,👿,👹,👺,💀,☠️,👻,👽,👾,🤖,💩";
	QStringList ovoList = ovo.split(",");
	int cnt = 0;
	for (int rowCount = 0; rowCount < 8; rowCount++)
	{
		for (int columnCount = 0; columnCount < 14; columnCount++)
		{
			QLabel* label = new QLabel(this);
			ovoMatrix[rowCount][columnCount] = ovoList[cnt++];
			label->setText(ovoMatrix[rowCount][columnCount]);
			label->setAlignment(Qt::AlignCenter);
			label->setFont(QFont("Segoe UI Emoji", 12));	//Arial
			label->setStyleSheet("padding: 0px;");
			ui->tableWidget->setCellWidget(rowCount, columnCount, label);
		}
	}
}

OvOWidget::~OvOWidget()
{
	delete selectionModel;
	delete ui;
}

void OvOWidget::OvOClicked(int row, int column)
{
	this->close();
	emit OvOChanged(ovoMatrix[row][column]);
}

void OvOWidget::ovoUpdate(const QModelIndex& current, const QModelIndex& previous)
{
	Q_UNUSED(current);
	Q_UNUSED(previous);
	//qDebug() << ovoMatrix[current.row()][current.column()];
	//emit OvOChanged(ovoMatrix[current.row()][current.column()]);
}
