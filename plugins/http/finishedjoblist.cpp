#include <QContextMenuEvent>
#include <QMenu>
#include "finishedjoblist.h"

FinishedJobList::FinishedJobList (QWidget *parent)
: QTreeWidget (parent)
{
}

void FinishedJobList::AddAction (QAction *action)
{
	Actions_.append (action);
}

void FinishedJobList::contextMenuEvent (QContextMenuEvent *e)
{
	QMenu menu (this);
	for (int i = 0; i < Actions_.size (); ++i)
		menu.addAction (Actions_.at (i));
	menu.exec (e->globalPos ());

	e->accept ();
}

