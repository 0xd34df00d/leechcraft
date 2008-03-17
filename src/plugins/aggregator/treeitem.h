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
    int ChildPosition (TreeItem*);
    TreeItem* Child (int);
    int ChildCount () const;
    int ColumnCount () const;
    QVariant Data (int) const;
    void ModifyData (int, const QVariant&);
    TreeItem* Parent ();
    int Row () const;
};

#endif

