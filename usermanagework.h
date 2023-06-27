#pragma once
#pragma execution_character_set("utf-8")

#ifndef USERMANAGEWORK_H
#define USERMANAGEWORK_H

#include <QObject>
#include "service.h"

class UserManageWork : public QObject
{
    Q_OBJECT
public:
    explicit UserManageWork(QObject* parent = nullptr);
    ~UserManageWork();
    void working(QSqlRelationalTableModel* model);
    void setHeartBeat(bool flag);
    //void setModel(QSqlRelationalTableModel* model);
    QSqlDatabase getDB();
    void submitAll();
    void setFilter(const QString& filter);
    void loadAvatar();
    void queryAccount(const QString& account);
    void setCurAvatarUrl(const QString& url);
    void setCombox(QComboBox* group, QComboBox* department);
    //void getComboxItems(QStringList& comboxItems_group, QStringList& comboxItems_department);   //已弃用
    void getVerify(const QString& uid);
    void updateVerify(int type, int verifyTag, const QString& info);    //type == 0 取消认证，1新增认证，2更新认证
    QString getVerifyInfo();
    QString getVerifyType();
    int getVerifyTag();
    QString getUid();
    bool isDisplay = false;
    QSqlRelationalTableModel* getModel();

private slots:

private:
    service db_service;
    QComboBox* m_group, * m_department;
    MySQL DB_SERVICE_SECOND;
    QSqlDatabase DB;
    QString avatarUrl, uid;
    QPixmap curPix;
    QSqlRelationalTableModel* relTableModel = nullptr;
    QStringList comboxItems_group, comboxItems_department;
    QString verifyInfo, verifyType;
    QTimer* heartBeat = nullptr;
    int verifyTag;
    void getComboxItems();
signals:
    void userManageWorkFinished();
    void queryAccountFinished(QSqlRecord);
    void submitAllFinished(bool);
    void avatarFinished(QPixmap curPix);
    void getVerifyFinished(bool res);
    void updateVerifyFinished(bool res);
};

#endif // USERMANAGEWORK_H