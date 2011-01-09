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

#include "syncbookmarks.h"
#include <plugininterface/util.h>
#include "core.h"
#include "abstractbookmarksservice.h"
#include "xmlsettingsmanager.h"

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
	SyncBookmarks::SyncBookmarks ()
	{
	}

	void SyncBookmarks::syncBookmarks ()
	{
	}

	void SyncBookmarks::uploadBookmarks ()
	{
	} 

	void SyncBookmarks::downloadBookmarks ()
	{
		QMap<AbstractBookmarksService *, QStringList> accountData;
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
		{
			service->DownloadBookmarks (XmlSettingsManager::Instance ()->
					property (("Account/" + service->GetName ()).toUtf8 ()).toStringList (), 
					XmlSettingsManager::Instance ()->property ("Sync/IsService2LocalLastSyncDate").toInt ());
			
			connect (service,
					SIGNAL (gotDownloadReply (const QList<QVariant>&, const QUrl&)),
					this,
					SLOT (readDownloadReply (const QList<QVariant>&, const QUrl&)));
			
			connect (service,
					SIGNAL (gotParseError (const QString&)),
					this,
					SLOT (readErrorReply (const QString&)));
			
		}
	}
	
	void SyncBookmarks::readDownloadReply (const QList<QVariant>& importBookmarks, const QUrl &url)
	{
		Entity eBookmarks = Util::MakeEntity (QVariant (),
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/browser-import-data");
				
		eBookmarks.Additional_ ["BrowserBookmarks"] = importBookmarks;
		emit gotEntity (eBookmarks);
	}
	
	void SyncBookmarks::readErrorReply(const QString& errorReply)
	{
		Entity e = Util::MakeNotification ("Poshuku", 
				errorReply, 
				PCritical_);
		
		gotEntity (e);
	}

}
}
}
}
}