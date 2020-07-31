/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_FAVORITESTREEVIEW_H
#define PLUGINS_POSHUKU_FAVORITESTREEVIEW_H
#include <QTreeView>

namespace LC
{
namespace Poshuku
{
	class FavoritesTreeView : public QTreeView
	{
		Q_OBJECT
	public:
		FavoritesTreeView (QWidget* = 0);
		virtual ~FavoritesTreeView ();
	protected:
		virtual void keyPressEvent (QKeyEvent*);
	signals:
		void deleteSelected (const QModelIndex&);
	};
}
}

#endif
