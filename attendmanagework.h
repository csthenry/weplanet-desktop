#ifndef ATTENDMANAGEWORK_H
#define ATTENDMANAGEWORK_H

#include <QObject>
#include "querymodel.h"

class attendManageWork : public QObject
{
    Q_OBJECT
public:
    explicit attendManageWork(QObject *parent = nullptr);

private:
    QSqlRelationalTableModel *userTableModel, *attendTableModel;
    void loadUserModel();
    void loadAttendModel();

signals:
    void sendUserModel(QSqlRelationalTableModel *userTableModel);
    void sendAttendModel(QSqlRelationalTableModel *attendTableModel);
};

#endif // ATTENDMANAGEWORK_H
