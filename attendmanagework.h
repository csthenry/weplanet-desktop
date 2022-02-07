#pragma once
#pragma execution_character_set("utf-8")

#ifndef ATTENDMANAGEWORK_H
#define ATTENDMANAGEWORK_H

#include <QObject>
#include "querymodel.h"
#include "service.h"

class AttendManageWork : public QObject
{
    Q_OBJECT
public:
    explicit AttendManageWork(QObject *parent = nullptr);
    bool isFirst = true;    //是否为首次加载model
    void working();
    void setDB(const QSqlDatabase& db);
    void setCurAvatarUrl(const QString url);
    void setUserModel(QSqlRelationalTableModel *relTableModel);
    void setAttendModel(QSqlRelationalTableModel *relTableModel);
    void getComboxItems(QStringList& comboxItems_group, QStringList& comboxItems_department);
    void submitAll(int type);   //1补签 0退签
    void loadAvatar();
private:
    QPixmap curPix;
    QString avatarUrl;
    QSqlDatabase DB;
    QStringList comboxItems_group, comboxItems_department;
    QSqlRelationalTableModel *userModel, *attendModel;
    void getComboxItems();
signals:
    void attendManageWorkFinished();
    void submitAddFinished(bool);
    void submitDelFinished(bool);
    void avatarFinished(QPixmap curPix);
};

#endif // ATTENDMANAGEWORK_H
