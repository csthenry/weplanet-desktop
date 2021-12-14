#include "mainwindow.h"
#include "formlogin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    formLogin *formLoginWindow = new formLogin();
    if (formLoginWindow->exec()==QDialog::Accepted)
    {
        delete formLoginWindow;
        MainWindow w;
        w.show();
        return a.exec();
    }
    else
        return  0;
    return a.exec();
}
