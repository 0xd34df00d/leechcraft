#ifndef CONTEXTABLELIST_H
#define CONTEXTABLELIST_H
#include <QTreeWidget>
#include <QList>

class ContextableList : public QTreeWidget
{
	Q_OBJECT
	
	QList<QAction*> Actions_;
public:
	ContextableList (QWidget *parent = 0);
	void AddAction (QAction*);
protected:
	virtual void contextMenuEvent (QContextMenuEvent*);
};

#endif

