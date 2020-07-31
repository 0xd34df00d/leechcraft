/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "favoritestreeview.h"
#include <QKeyEvent>

namespace LC
{
namespace Poshuku
{
	FavoritesTreeView::FavoritesTreeView (QWidget *parent)
	: QTreeView (parent)
	{
	}
	
	FavoritesTreeView::~FavoritesTreeView ()
	{
	}
	
	void FavoritesTreeView::keyPressEvent (QKeyEvent *e)
	{
		if (e->key () == Qt::Key_Delete &&
				selectionModel ()->currentIndex ().isValid ())
			emit deleteSelected (selectionModel ()->currentIndex ());
	
		QTreeView::keyPressEvent (e);
	}
}
}
