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
#include <QDateTime>
#include <QMetaObject>
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
	SyncBookmarks::SyncBookmarks (QObject *parent)
	{
	}

	void SyncBookmarks::syncBookmarks ()
	{
	}

	void SyncBookmarks::uploadBookmarks ()
	{
		QList<QVariant> result;

		if (!QMetaObject::invokeMethod (Core::Instance ().GetBookmarksModel (),
					"getItemsMap",
					Q_RETURN_ARG ( QList<QVariant>, result)))
		{
			qWarning () << Q_FUNC_INFO
					<< "getItemsMap() metacall failed"
					<< result;
			return;
		}

		QMap<AbstractBookmarksService*, QStringList> accountData;
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
		{
			service->UploadBookmarks (XmlSettingsManager::Instance ()->
				property (("Account/" + service->GetName ()).toUtf8 ()).toStringList (),
				result);

			connect (service,
					SIGNAL (gotUploadReply (bool)),
					this,
					SLOT (readUploadReply (bool)),
					Qt::UniqueConnection);

			connect (service,
					SIGNAL (gotParseError (const QString&)),
					this,
					SLOT (readErrorReply (const QString&)),
					Qt::UniqueConnection);
		}
	}

	void SyncBookmarks::downloadBookmarks ()
	{
		QMap<AbstractBookmarksService*, QStringList> accountData;
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
		{
			service->DownloadBookmarks (XmlSettingsManager::Instance ()->
					property (("Account/" + service->GetName ()).toUtf8 ()).toStringList (),
					XmlSettingsManager::Instance ()->
					property ((service->GetName () + "/LastDownload").toUtf8 ()).toInt ());

			connect (service,
					SIGNAL (gotDownloadReply (const QList<QVariant>&, const QUrl&)),
					this,
					SLOT (readDownloadReply (const QList<QVariant>&, const QUrl&)),
					Qt::UniqueConnection);

			connect (service,
					SIGNAL (gotParseError (const QString&)),
					this,
					SLOT (readErrorReply (const QString&)),
					Qt::UniqueConnection);
		}
	}

	void SyncBookmarks::readDownloadReply (const QList<QVariant>& importBookmarks, const QUrl &url)
	{
		Entity eBookmarks = Util::MakeEntity (QVariant (),
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/browser-import-data");

		AbstractBookmarksService *service = qobject_cast<AbstractBookmarksService*> (sender ());
		if (!service)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a AbstractBookmarksService"
					<< sender ();
			return;
		}

		XmlSettingsManager::Instance ()->
				setProperty ((service->GetName () + "/LastDownload").toUtf8 (),
				QDateTime::currentDateTime ().toTime_t ());

		eBookmarks.Additional_ ["BrowserBookmarks"] = importBookmarks;
		Core::Instance ().SendEntity (eBookmarks);
	}

	void SyncBookmarks::readUploadReply (bool success)
	{
		if (!success)
		{
			Core::Instance ().SendEntity (Util::MakeNotification ("Poshuku",
						tr ("Error while sending bookmarks"),
						PCritical_));
			return;
		}

		AbstractBookmarksService *service = qobject_cast<AbstractBookmarksService*> (sender ());
		if (!service)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a AbstractBookmarksService"
					<< sender ();
			return;
		}

		XmlSettingsManager::Instance ()->
				setProperty ((service->GetName () + "/LastUpload").toUtf8 (),
				QDateTime::currentDateTime ().toTime_t ());

		Core::Instance ().SendEntity (Util::MakeNotification ("Poshuku",
					tr ("Bookmarks sent successfully"),
					PInfo_));
	}

	void SyncBookmarks::readErrorReply (const QString& errorReply)
	{
		Core::Instance ().SendEntity (Util::MakeNotification ("Poshuku",
					errorReply,
					PCritical_));
	}
}
}
}
}
}