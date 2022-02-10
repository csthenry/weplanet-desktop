#pragma once
#pragma execution_character_set("utf-8")

#ifndef USERMANAGEWORK_H
#define USERMANAGEWORK_H

#include <QObject>
#include "querymodel.h"
#include "service.h"

class UserManageWork : public QObject
{
    Q_OBJECT
public:
    bool isFirst = true;
    explicit UserManageWork(QObject *parent = nullptr);
    void working();
    void setModel(QSqlRelationalTableModel* model);
    QSqlDatabase getDB();
    void submitAll();
    void loadAvatar();
    void setUsersTypeCombox();
    void setCurAvatarUrl(const QString& url);
    void setCombox(QComboBox* group, QComboBox* department);
    void getComboxItems(QStringList& comboxItems_group, QStringList& comboxItems_department);   //已弃用
private slots:

private:
    service db_service;
    QComboBox *m_group, *m_department;
    QSqlDatabase DB;
    QString avatarUrl;
    QPixmap curPix;
    QSqlRelationalTableModel *relTableModel;
    QStringList comboxItems_group, comboxItems_department;
    void getComboxItems();
signals:
    void userManageWorkFinished();
    void submitAllFinished(bool);
    void avatarFinished(QPixmap curPix);
};

#endif // USERMANAGEWORK_H
