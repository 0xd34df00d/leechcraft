/*******************************************************************
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SYNCBOOKMARKS_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SYNCBOOKMARKS_H

#include <QObject>
#include "interfaces/structures.h"

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
	class AbstractBookmarksService;
	
	class SyncBookmarks : public QObject
	{
		Q_OBJECT
		
		bool IsSync_;
	public:
		SyncBookmarks (QObject *parent = 0);
		bool IsUrlInUploadFile (const QString&);
	public slots:
		void syncBookmarks ();
		void uploadBookmarksAction (const QString& title = QString (), const QString& url = QString (), 
				const QStringList& tags = QStringList (), AbstractBookmarksService *as = 0);
		void downloadBookmarksAction ();
		void downloadAllBookmarksAction ();
		void readDownloadReply (const QList<QVariant>&, const QUrl&);
		void readUploadReply (bool);
		void readErrorReply (const QString&);
		void CheckDownloadPeriod ();
		void CheckUploadPeriod ();
	private slots:
		void downloadBookmarks (AbstractBookmarksService*, uint);
	private:
		QList<QVariant> GetBookmarksForUpload (const QString& url = QString ());
		QStringList GetUrlsFromUploadFile () const;
	};
}
}
}
}
}
#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SYNCBOOKMARKS_H
