/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_BOOKMARKSDIALOG_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_BOOKMARKSDIALOG_H
#include <QDialog>
#include "ui_bookmarksdialog.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class BookmarksDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::BookmarksDialog_ Ui_;
	public:
		BookmarksDialog (QWidget* parent = 0, Qt::WindowFlags f = 0);
		void SetBookmark (const QString&, const QString&, const QStringList&);
		void SendBookmark ();
	public slots:
		void sendBookmarkWithoutConfirm (bool);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_BOOKMARKSDIALOG_H
