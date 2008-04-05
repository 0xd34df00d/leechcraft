#include <QContextMenuEvent>
#include <QMenu>
#include "contextableview.h"

ContextableView::ContextableView (QWidget *parent)
: QTreeView (parent)
{
}

void ContextableView::AddAction (QAction *action)
{
    Actions_.append (action);
}

void ContextableView::contextMenuEvent (QContextMenuEvent *e)
{
    QMenu menu (this);
    for (int i = 0; i < Actions_.size (); ++i)
        menu.addAction (Actions_.at (i));
    menu.exec (e->globalPos ());

    e->accept ();
}

