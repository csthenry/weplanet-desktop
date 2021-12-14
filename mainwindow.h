#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QPixmap>
#include <QLabel>
#include "service.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    QLabel *connectStatusLable, *statusIcon;

    QSqlTableModel *tabModel;  //数据模型

    QItemSelectionModel *theSelection; //选择模型

    QDataWidgetMapper *dataMapper; //数据映射

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
