#include <QStringList>
#include "treeitem.h"

TreeItem::TreeItem (const QList<QVariant>& data, TreeItem *parent)
: Data_ (data)
, Parent_ (parent)
{
}

TreeItem::~TreeItem ()
{
    qDeleteAll (Childs_);
}

void TreeItem::AppendChild (TreeItem *child)
{
    Childs_.append (child);
}

void TreeItem::PrependChild (TreeItem *child)
{
    Childs_.prepend (child);
}

int TreeItem::ChildPosition (TreeItem *child)
{
    return Childs_.indexOf (child);
}

void TreeItem::RemoveChild (int child)
{
    Childs_.removeAt (child);
}

TreeItem* TreeItem::Child (int row)
{
    return Childs_.value (row);
}

int TreeItem::ChildCount () const
{
    return Childs_.count ();
}

int TreeItem::ColumnCount () const
{
    return Data_.count ();
}

QVariant TreeItem::Data (int column) const
{
    return Data_.value (column);
}

void TreeItem::ModifyData (int column, const QVariant& data)
{
    if (Data_.size () <= column)
        return;
    Data_ [column] = data;
}

TreeItem* TreeItem::Parent ()
{
    return Parent_;
}

int TreeItem::Row () const
{
    if (Parent_)
        return Parent_->Childs_.indexOf (const_cast<TreeItem*> (this));
    return 0;
}

