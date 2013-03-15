/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QWidget>
#include "interfaces/monocle/idocument.h"
#include "ui_bookmarkswidget.h"

class QStandardItemModel;
class QToolBar;

namespace LeechCraft
{
namespace Monocle
{
	class DocumentTab;
	class Bookmark;

	class BookmarksWidget : public QWidget
	{
		Q_OBJECT

		Ui::BookmarksWidget Ui_;
		DocumentTab *Tab_;
		QToolBar *Toolbar_;

		QStandardItemModel *BMModel_;

		IDocument_ptr Doc_;
	public:
		BookmarksWidget (DocumentTab*, QWidget* = 0);

		void HandleDoc (IDocument_ptr);
	private:
		void AddBMToTree (const Bookmark&);
	private slots:
		void handleAddBookmark ();
		void on_BookmarksView__activated (const QModelIndex&);
	};
}
}
