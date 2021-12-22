#ifndef DELEGATE_H
#define DELEGATE_H
#include <QItemDelegate>

class readOnlyDelegate : public QItemDelegate
{
public:
    readOnlyDelegate(QWidget *parent = NULL);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

};

#endif // DELEGATE_H
