#include <QSpinBox>
#include <QtDebug>
#include <QModelIndex>
#include <plugininterface/treeitem.h>
#include "filesviewdelegate.h"

using LeechCraft::Util::TreeItem;

FilesViewDelegate::FilesViewDelegate (QObject *parent)
: QItemDelegate (parent)
{
}

FilesViewDelegate::~FilesViewDelegate ()
{
}

QWidget* FilesViewDelegate::createEditor (QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column () == 2)
    {
        QSpinBox *box = new QSpinBox (parent);
        box->setRange (0, 7);
        return box;
    }
    else
        return QItemDelegate::createEditor (parent, option, index);
}

void FilesViewDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QItemDelegate::paint (painter, option, index);
}

void FilesViewDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
{
    if (index.column () == 2)
        qobject_cast<QSpinBox*> (editor)->setValue (static_cast<TreeItem*> (index.internalPointer ())->Data (2).toInt ());
    else
        QItemDelegate::setEditorData (editor, index);
}

void FilesViewDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
{
    if (index.column () == 2)
    {
		if (model->rowCount (index.sibling (index.row (), 0)))
			return;
        int value = qobject_cast<QSpinBox*> (editor)->value ();
        model->setData (index, value);
    }
    else
        QItemDelegate::setModelData (editor, model, index);
}

