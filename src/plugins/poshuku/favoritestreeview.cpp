#include "favoritestreeview.h"
#include <QKeyEvent>

FavoritesTreeView::FavoritesTreeView (QWidget *parent)
: QTreeView (parent)
{
}

FavoritesTreeView::~FavoritesTreeView ()
{
}

void FavoritesTreeView::keyPressEvent (QKeyEvent *e)
{
	if (e->key () == Qt::Key_Delete)
		emit deleteSelected (selectionModel ()->currentIndex ());

	QTreeView::keyPressEvent (e);
}

