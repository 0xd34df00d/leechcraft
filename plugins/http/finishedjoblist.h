#ifndef FINISHEDJOBLIST_H
#define FINISHEDJOBLIST_H
#include <QTreeWidget>
#include <QList>

class FinishedJobList : public QTreeWidget
{
	Q_OBJECT
	
	QList<QAction*> Actions_;
public:
	FinishedJobList (QWidget *parent = 0);
	void AddAction (QAction*);
protected:
	virtual void contextMenuEvent (QContextMenuEvent*);
};

#endif

