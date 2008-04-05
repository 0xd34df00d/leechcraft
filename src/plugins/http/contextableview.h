#ifndef CONTEXTABLEVIEW_H
#define CONTEXTABLEVIEW_H
#include <QTreeView>
#include <QList>

class ContextableView : public QTreeView
{
    Q_OBJECT
    
    QList<QAction*> Actions_;
public:
    ContextableView (QWidget *parent = 0);
    void AddAction (QAction*);
protected:
    virtual void contextMenuEvent (QContextMenuEvent*);
};

#endif

