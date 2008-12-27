#ifndef HISTORYTREEVIEW_H
#define HISTORYTREEVIEW_H
#include <QTreeView>

class HistoryTreeView : public QTreeView
{
	Q_OBJECT
public:
	HistoryTreeView (QWidget* = 0);
protected:
	virtual void keyPressEvent (QKeyEvent*);
signals:
	void deleteSelected (const QModelIndex&);
};

#endif

