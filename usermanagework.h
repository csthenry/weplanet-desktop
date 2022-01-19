#ifndef USERMANAGEWORK_H
#define USERMANAGEWORK_H

#include <QObject>
#include "querymodel.h"
#include "service.h"

class UserManageWork : public QObject
{
    Q_OBJECT
public:
    explicit UserManageWork(QObject *parent = nullptr);
    void working();
    void setDB(const QSqlDatabase& DB);
    void setModel(QSqlRelationalTableModel* model);
    void submitAll();
    void loadAvatar();
    void setUsersTypeCombox();
    void setCurAvatarUrl(const QString& url);
private slots:

private:
    QSqlDatabase DB;
    QString avatarUrl;
    QPixmap curPix;
    QSqlRelationalTableModel *relTableModel;
    QStringList comboxItems_group, comboxItems_department;
signals:
    void userManageWorkFinished();
    void submitAllFinished(bool);
    void avatarFinished(QPixmap curPix);
    void comboxSetFinished(QStringList comboxItems_group, QStringList comboxItems_department);
};

#endif // USERMANAGEWORK_H
