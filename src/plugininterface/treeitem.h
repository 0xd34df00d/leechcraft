#ifndef TREEITEM_H
#define TREEITEM_H
#include <QList>
#include <QVector>
#include <QMap>
#include <QVariant>

class TreeItem
{
    QList<TreeItem*> Childs_;
    QMap<int, QVector<QVariant> > Data_;
    TreeItem *Parent_;
public:
    TreeItem (const QList<QVariant>&, TreeItem *parent = 0);
    ~TreeItem ();

    void AppendChild (TreeItem*);
    void PrependChild (TreeItem*);
    int ChildPosition (TreeItem*);
    void RemoveChild (int);
    TreeItem* Child (int);
    int ChildCount () const;
    int ColumnCount (int role = Qt::DisplayRole) const;
    QVariant Data (int, int role = Qt::DisplayRole) const;
    void ModifyData (int, const QVariant&, int role = Qt::DisplayRole);
    TreeItem* Parent ();
    int Row () const;
};

#endif

