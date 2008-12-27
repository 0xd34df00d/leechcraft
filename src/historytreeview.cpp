#include "historytreeview.h"
#include <QKeyEvent>

HistoryTreeView::HistoryTreeView (QWidget *parent)
: QTreeView (parent)
{
}

void HistoryTreeView::keyPressEvent (QKeyEvent *e)
{
	if (e->key () == Qt::Key_Delete)
		emit deleteSelected (selectionModel ()->currentIndex ());

	QTreeView::keyPressEvent (e);
}

