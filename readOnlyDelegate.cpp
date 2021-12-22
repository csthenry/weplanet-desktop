#include "readOnlyDelegate.h"


readOnlyDelegate::readOnlyDelegate(QWidget *parent):QItemDelegate(parent)
{

}

QWidget *readOnlyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const //final
{
    Q_UNUSED(parent)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return NULL;
}
