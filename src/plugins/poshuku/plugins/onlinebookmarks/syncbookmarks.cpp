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
#include <sys/socket.h>

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
	: IsSync_ (false)
	{
	}

	void SyncBookmarks::syncBookmarks ()
	{
		downloadBookmarksAction ();
		IsSync_ = true;
	}

	void SyncBookmarks::uploadBookmarksAction (const QString& title, const QString& url, const QStringList& tags)
	{
		QList<QVariant> result;
		QMap<QString, QVariant> link;
		link ["Title"] = title;
		link ["URL"] = url;
		link ["Tags"] = tags;
		
		result = GetBookmarksForUpload (url);
		
		QMap<AbstractBookmarksService*, QStringList> accountData;
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
		{
			service->UploadBookmarks (XmlSettingsManager::Instance ()->
				property ("Account/" + service->GetName ().toUtf8 ().toBase64 ()).toStringList (),
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
					
			connect (service,
					SIGNAL (gotInformationMessage (const QString&)),
					this,
					SLOT (readInformationMessage (const QString&)));
		}
	} 

	void SyncBookmarks::downloadBookmarksAction ()
	{
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
			downloadBookmarks(service, XmlSettingsManager::Instance ()->
					Property (service->GetName ().toUtf8 ().toBase64 () + "/LastDownload", 
					QVariant (QDateTime::fromString ("01.01.1970", "dd.MM.yyyy").toTime_t ())).toInt ());
	}

	void SyncBookmarks::downloadAllBookmarksAction ()
	{
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
			downloadBookmarks(service, QDateTime::fromString ("01.01.1970", "dd.MM.yyyy").toTime_t ());
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
		
		QDir dir = Core::Instance ().GetBookmarksDir ();
		QFile file (dir.absolutePath () + "/uploadBookmarks");
		
		if (!file.open (QIODevice::ReadWrite))
		{
			Entity e = Util::MakeNotification ("Poshuku", 
					tr ("Unable to open upload configuration file."), 
					PCritical_);
			
			Core::Instance ().SendEntity (e);
			return;
		}
		
		const QByteArray& data = file.readAll ();
		if (data.isEmpty ())
		{
			QStringList urls;
			Q_FOREACH (const QVariant& var, importBookmarks)
				urls << var.toMap () ["URL"].toString ();
			
			file.write (urls.join ("\n").toUtf8 ());
			file.write ("\n");
		}
		else
		{
			QStringList urls = QString::fromUtf8 (data).split ('\n');
			QStringList newBookmarksUrl;
			
			Q_FOREACH (const QVariant& var, importBookmarks)
			{
				QString currentUrl = var.toMap () ["URL"].toString ();
				if (!urls.contains (currentUrl, Qt::CaseInsensitive))
					newBookmarksUrl << currentUrl;
			}
			file.write (newBookmarksUrl.join ("\n").toUtf8 ());
			file.write ("\n");
		}
		file.close ();
		
		XmlSettingsManager::Instance ()->
				setProperty (service->GetName ().toUtf8 ().toBase64 () + "/LastDownload", 
				QDateTime::currentDateTime ().toTime_t ());
		
		eBookmarks.Additional_ ["BrowserBookmarks"] = importBookmarks;
		Core::Instance ().SendEntity (eBookmarks);
		
		if (IsSync_)
		{
			uploadBookmarksAction ();
			IsSync_ = false;
		}
	}
	
	void SyncBookmarks::readUploadReply (bool success)
	{
		Entity e;
		
		if (!success)
		{
			e = Util::MakeNotification ("Poshuku", 
				tr ("Error while sending bookmarks"), 
				PCritical_);
		
			Core::Instance ().SendEntity (e);
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
				setProperty (service->GetName ().toUtf8 ().toBase64 () + "/LastUpload", 
				QDateTime::currentDateTime ().toTime_t ());

		e = Util::MakeNotification ("Poshuku", 
				tr ("Bookmarks have been sent successfully"), 
				PInfo_);
		
		Core::Instance ().SendEntity (e);
	}
	
	void SyncBookmarks::readErrorReply (const QString& errorReply)
	{
		Entity e = Util::MakeNotification ("Poshuku", 
				errorReply, 
				PCritical_);
		
		Core::Instance ().SendEntity (e);
	}
	
	QList<QVariant> SyncBookmarks::GetBookmarksForUpload (const QString& url)
	{
		QList<QVariant> result;
		if (url.isEmpty ())
		{
			if (!QMetaObject::invokeMethod (Core::Instance ().GetBookmarksModel (), 
						"getItemsMap", 
						Q_RETURN_ARG (QList<QVariant>, result)))
			{
				qWarning () << Q_FUNC_INFO
						<< "getItemsMap() metacall failed"
						<< result;
				return QList<QVariant> ();
			}
		}
		else
		{
			QMap <QString, QVariant> link;
			link ["URL"] = url;
			result << link;
		}
		
		QDir dir = Core::Instance ().GetBookmarksDir ();
		QFile file (dir.absolutePath () + "/uploadBookmarks");
		
		if (!file.open (QIODevice::ReadWrite))
		{
			Entity e = Util::MakeNotification ("Poshuku", 
					tr ("Unable to open upload configuration file."), 
					PCritical_);
			
			Core::Instance ().SendEntity (e);
			return result;
		}
		
		const QByteArray& data = file.readAll ();
		if (data.isEmpty ())
		{
			QStringList urls;
			Q_FOREACH (const QVariant& var, result)
				urls << var.toMap () ["URL"].toString ();
			
			file.write (urls.join ("\n").toUtf8 ());
			file.write ("\n");
			file.close ();
		}
		else
		{
			QStringList urls = QString::fromUtf8 (data).split ('\n');
			QList<QVariant> newBookmarks;
			QStringList newBookmarksUrl;
			
			Q_FOREACH (const QVariant& var, result)
			{
				QString currentUrl = var.toMap () ["URL"].toString ();
				if (!urls.contains (currentUrl, Qt::CaseInsensitive))
				{
					newBookmarks << var;
					newBookmarksUrl << currentUrl;
				}
			}
			file.write (newBookmarksUrl.join ("\n").toUtf8 ());
			file.write ("\n");
			file.close ();
			
			return newBookmarks;
		}
		
		return result;
	}
	
	void SyncBookmarks::downloadBookmarks (AbstractBookmarksService *service, uint fromTime)
	{
		service->DownloadBookmarks (XmlSettingsManager::Instance ()->
					property ("Account/" + service->GetName ().toUtf8 ().toBase64 ()).toStringList (), 
					fromTime);
			
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
}
}
}
}
