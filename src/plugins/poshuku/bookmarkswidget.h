/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include "filtermodel.h"
#include "ui_bookmarkswidget.h"

namespace LC
{
namespace Util
{
	class FlatToFoldersProxyModel;
}

namespace Poshuku
{
	class BookmarksWidget : public QWidget
	{
		Q_OBJECT

		Ui::BookmarksWidget Ui_;
		std::shared_ptr<Util::FlatToFoldersProxyModel> FlatToFolders_;
		std::unique_ptr<FilterModel> FavoritesFilterModel_;
	public:
		BookmarksWidget (QWidget* = 0);
	private slots:
		void on_ActionEditBookmark__triggered ();
		void on_ActionDeleteBookmark__triggered ();
		void translateRemoveFavoritesItem (const QModelIndex&);
		void updateFavoritesFilter ();
		void on_FavoritesView__activated (const QModelIndex&);
		void on_OpenInTabs__released ();
		void selectTagsMode ();
		void handleGroupBookmarks ();
	};
}
}
