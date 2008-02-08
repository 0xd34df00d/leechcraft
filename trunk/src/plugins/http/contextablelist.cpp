#include <QContextMenuEvent>
#include <QMenu>
#include "contextablelist.h"

ContextableList::ContextableList (QWidget *parent)
: QTreeWidget (parent)
{
}

void ContextableList::AddAction (QAction *action)
{
   Actions_.append (action);
}

void ContextableList::contextMenuEvent (QContextMenuEvent *e)
{
   QMenu menu (this);
   for (int i = 0; i < Actions_.size (); ++i)
      menu.addAction (Actions_.at (i));
   menu.exec (e->globalPos ());

   e->accept ();
}

