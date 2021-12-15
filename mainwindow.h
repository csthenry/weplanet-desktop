#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QPixmap>
#include <QLabel>
#include "service.h"
#include "formlogin.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QString uid;

    QSqlDatabase db;

    QLabel *connectStatusLable, *statusIcon;

    QSqlTableModel *tabModel;  //数据模型

    QItemSelectionModel *theSelection; //选择模型

    QDataWidgetMapper *dataMapper; //数据映射

    QPixmap statusOKIcon, statusErrorIcon;

    formLogin *formLoginWindow;
public:
    MainWindow(QWidget *parent = nullptr, QDialog *formLoginWindow = nullptr);
    ~MainWindow();


private slots:
    void receiveData(QSqlDatabase db, QString uid); //接收登录窗口信号

    void on_actExit_triggered();

    void on_actHome_triggered();

    void on_actMyInfo_triggered();

    void on_actAttend_triggered();

    void on_actApply_triggered();

    void on_actUserManager_triggered();

    void on_actAttendManager_triggered();

    void on_actApplyList_triggered();

    void on_actApplyItems_triggered();

    void on_actGroup_triggered();

    void on_actMore_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
