/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
#include <QObject>
#include <interfaces/structures.h>

class QStandardItemModel;
class QNetworkAccessManager;

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace OnlineBookmarks
{
	class SyncBookmarks;
	class AbstractBookmarksService;
	
	class Core : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		QNetworkAccessManager *Manager_;
		SyncBookmarks *BookmarksSyncManager_;
		QList<AbstractBookmarksService*> ActiveBookmarksServices_;
		
		Core ();
	public:
		static Core& Instance ();
		void Init ();
		void SendEntity (const Entity&);
		QStandardItemModel* GetAccountModel () const;
		SyncBookmarks* GetBookmarksSyncManager () const;
		void SetActiveBookmarksServices (QList<AbstractBookmarksService*>);
		QList<AbstractBookmarksService*> GetActiveBookmarksServices () const;
		void SetPassword (const QString&, const QString&, const QString&);
		QString GetPassword (const QString&, const QString&) const;
		void SetNetworkAccessManager (QNetworkAccessManager *manager);
		QNetworkAccessManager* GetNetworkAccessManager () const;
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H

