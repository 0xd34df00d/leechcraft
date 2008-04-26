#ifndef FILESVIEWDELEGATE_H
#define FILESVIEWDELEGATE_H
#include <QItemDelegate>

class FilesViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    FilesViewDelegate (QObject *parent = 0);
    virtual ~FilesViewDelegate ();

    virtual QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
    virtual void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
    virtual void setEditorData (QWidget*, const QModelIndex&) const;
    virtual void setModelData (QWidget*, QAbstractItemModel*, const QModelIndex&) const;
};

#endif

