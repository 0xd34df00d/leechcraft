#ifndef TREEITEM_H
#define TREEITEM_H
#include <QList>
#include <QVariant>

class TreeItem
{
    QList<TreeItem*> Childs_;
    QList<QVariant> Data_;
    TreeItem *Parent_;
public:
    TreeItem (const QList<QVariant>&, TreeItem *parent = 0);
    ~TreeItem ();

    void AppendChild (TreeItem*);
    void PrependChild (TreeItem*);
    TreeItem* Child (int);
    int ChildCount () const;
    int ColumnCount () const;
    QVariant Data (int) const;
    TreeItem* Parent ();
    int Row () const;
};

#endif

