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
#include <QTimer>
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
	: IsSync_ (false)
	{
	}

	void SyncBookmarks::syncBookmarks ()
	{
		downloadBookmarksAction ();
		IsSync_ = true;
	}

	bool SyncBookmarks::IsUrlInUploadFile (const QString& url)
	{
		const QStringList& urls = GetUrlsFromUploadFile();
		Q_FOREACH (const QVariant& var, urls)
			if (urls.contains (url, Qt::CaseInsensitive))
				return true;
		return false;
	}

	void SyncBookmarks::uploadBookmarksAction (const QString& title, const QString& url, const QStringList& tags, 
			AbstractBookmarksService *as)
	{
		QList<QVariant> result;
		QMap<QString, QVariant> link;
		link ["Title"] = title;
		link ["URL"] = url;
		link ["Tags"] = tags;
		
		result = GetBookmarksForUpload (url);
		
		QMap<AbstractBookmarksService*, QStringList> accountData;
		QList<AbstractBookmarksService*> serviceList;
		
		if (as)
			serviceList << as;
		else
			serviceList = Core::Instance ().GetActiveBookmarksServices ();
		
		Q_FOREACH (AbstractBookmarksService *service, serviceList)
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
		}
	}

	void SyncBookmarks::downloadBookmarksAction ()
	{
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
			downloadBookmarks(service, XmlSettingsManager::Instance ()->
					Property (service->GetName ().toUtf8 ().toBase64 () + "/LastDownload", 
					QDateTime::fromString ("01.01.1970", "dd.MM.yyyy")).toDateTime ());
	}

	void SyncBookmarks::downloadAllBookmarksAction ()
	{
		Q_FOREACH (AbstractBookmarksService *service, Core::Instance ().GetActiveBookmarksServices ())
			downloadBookmarks(service, QDateTime::fromString ("01.01.1970", "dd.MM.yyyy"));
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
			if (!urls.isEmpty ())
			{
				file.write (urls.join ("\n").toUtf8 ());
				file.write ("\n");
			}
		}
		else
		{
			QStringList urls = Core::Instance ()
					.SanitizeTagsList (QString::fromUtf8 (data)
					.split ('\n', QString::SkipEmptyParts));
			
				
			QStringList newBookmarksUrl;
			
			Q_FOREACH (const QVariant& var, importBookmarks)
			{
				QString currentUrl = var.toMap () ["URL"].toString ();
				if (!urls.contains (currentUrl, Qt::CaseInsensitive))
					newBookmarksUrl << currentUrl;
			}
			if (!newBookmarksUrl.isEmpty ())
			{
				file.write (newBookmarksUrl.join ("\n").toUtf8 ());
				file.write ("\n");
			}
		}
		file.close ();
		
		XmlSettingsManager::Instance ()->
				setProperty (service->GetName ().toUtf8 ().toBase64 () + "/LastDownload", 
				QDateTime::currentDateTime ());
		
		eBookmarks.Additional_ ["BrowserBookmarks"] = importBookmarks;
		Core::Instance ().SendEntity (eBookmarks);
		
		if (IsSync_)
		{
			uploadBookmarksAction ();
			IsSync_ = false;
		}
		
		if (XmlSettingsManager::Instance ()->property ("DownloadGroup").toBool () && 
				XmlSettingsManager::Instance ()->property ("DownloadPeriod").toInt ())
		{
			int time = XmlSettingsManager::Instance ()->property ("DownloadPeriod").toInt () * 86400;
			QTimer::singleShot (time * 1000, this, SLOT (CheckDownloadPeriod ()));
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
				QDateTime::currentDateTime ());

		e = Util::MakeNotification ("Poshuku", 
				tr ("Bookmarks have been sent successfully"), 
				PInfo_);
		
		Core::Instance ().SendEntity (e);
		
		if (XmlSettingsManager::Instance ()->property ("UploadGroup").toBool () && 
				XmlSettingsManager::Instance ()->property ("UploadPeriod").toInt ())
		{
			uint time = XmlSettingsManager::Instance ()->property ("UploadPeriod").toInt () * 86400;
			QTimer::singleShot (time * 1000, this, SLOT (
					uploadBookmarksAction (QString (), QString (), QString (), sender ())));
		}
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
			
			if (!urls.isEmpty ())
			{
				file.write (urls.join ("\n").toUtf8 ());
				file.write ("\n");
			}
			file.close ();
		}
		else
		{
			QStringList urls = Core::Instance ()
					.SanitizeTagsList (QString::fromUtf8 (data)
					.split ('\n', QString::SkipEmptyParts));
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
			if (!newBookmarksUrl.isEmpty ())
			{
				file.write (newBookmarksUrl.join ("\n").toUtf8 ());
				file.write ("\n");
			}
			file.close ();
			
			return newBookmarks;
		}
		
		return result;
	}
	
	void SyncBookmarks::downloadBookmarks (AbstractBookmarksService *service, QDateTime fromTime)
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

	QStringList SyncBookmarks::GetUrlsFromUploadFile () const
	{
		QDir dir = Core::Instance ().GetBookmarksDir ();
		QFile file (dir.absolutePath () + "/uploadBookmarks");
		
		if (!file.open (QIODevice::ReadOnly))
		{
			Entity e = Util::MakeNotification ("Poshuku", 
					tr ("Unable to open upload configuration file."), 
					PCritical_);
			
			Core::Instance ().SendEntity (e);
			return QStringList ();
		}
		
		const QByteArray& data = file.readAll ();
		
		return Core::Instance ()
				.SanitizeTagsList (QString::fromUtf8 (data)
				.split ('\n', QString::SkipEmptyParts));
	}
	
	void SyncBookmarks::CheckDownloadPeriod ()
	{
		Q_FOREACH (AbstractBookmarksService *as, Core::Instance ().GetActiveBookmarksServices ())
		{
			QDateTime lastDownload = XmlSettingsManager::Instance ()->
					Property(as->GetName().toUtf8 ().toBase64 () + "/LastDownload",
					QDateTime::fromString ("01.01.1970", "dd.MM.yyyy")).toDateTime ();
			int diff = lastDownload.secsTo (QDateTime::currentDateTime ());
			
			if (diff >= XmlSettingsManager::Instance ()->
					property ("DownloadPeriod").toInt () * 86400)
					downloadBookmarks (as, lastDownload);
			else
				QTimer::singleShot (diff * 1000, this, SLOT (downloadBookmarks (as, lastDownload)));
		}
	}

	void SyncBookmarks::CheckUploadPeriod ()
	{
		Q_FOREACH (AbstractBookmarksService *as, Core::Instance ().GetActiveBookmarksServices ())
		{
			QDateTime lastUpload = XmlSettingsManager::Instance ()->
					Property (as->GetName ().toUtf8 ().toBase64 () + "/LastUpload",
					QDateTime::fromString ("01.01.1970", "dd.MM.yyyy")).toDateTime ();
			int diff = lastUpload.secsTo (QDateTime::currentDateTime ());
			
			if (diff >= XmlSettingsManager::Instance ()->
					property ("UpPeriod").toInt () * 86400)
				uploadBookmarksAction (QString (), QString (), QStringList(), as);
			else
				QTimer::singleShot (diff * 1000, this, SLOT (uploadBookmarksAction (QString (), QString (), QStringList(), as));
		}
	}
}
}
}
}
}
