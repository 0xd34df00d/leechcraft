#include <QContextMenuEvent>
#include <QMenu>
#include "contextabletree.h"

ContextableTree::ContextableTree (QWidget *parent)
: QTreeWidget (parent)
{
}

void ContextableTree::AddAction (QAction *action)
{
   Actions_.append (action);
}

void ContextableTree::contextMenuEvent (QContextMenuEvent *e)
{
   QMenu menu (this);
   for (int i = 0; i < Actions_.size (); ++i)
      menu.addAction (Actions_.at (i));
   menu.exec (e->globalPos ());

   e->accept ();
}

