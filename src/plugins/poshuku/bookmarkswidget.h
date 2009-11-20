/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_BOOKMARKSWIDGET_H
#define PLUGINS_POSHUKU_BOOKMARKSWIDGET_H
#include <memory>
#include <QWidget>
#include <plugininterface/tagscompleter.h>
#include "filtermodel.h"
#include "ui_bookmarkswidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class BookmarksWidget : public QWidget
			{
				Q_OBJECT

				Ui::BookmarksWidget Ui_;
				std::auto_ptr<FilterModel> FavoritesFilterModel_;
				std::auto_ptr<Util::TagsCompleter> FavoritesFilterLineCompleter_;
			public:
				BookmarksWidget (QWidget* = 0);
			private slots:
				void on_ActionEditBookmark__triggered ();
				void on_ActionChangeURL__triggered ();
				void on_ActionDeleteBookmark__triggered ();
				void translateRemoveFavoritesItem (const QModelIndex&);
				void updateFavoritesFilter ();
				void on_FavoritesView__activated (const QModelIndex&);
				void on_OpenInTabs__released ();
			};
		};
	};
};

#endif

