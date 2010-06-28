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

#include "networkdiskcache.h"
#include <QtDebug>
#include <QDateTime>
#include <QDir>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QTimer>
#include <QDirIterator>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	NetworkDiskCache::NetworkDiskCache (QObject *parent)
	: QNetworkDiskCache (parent)
	, IsCollectingGarbage_ (false)
	, PreviousSize_ (-1)
	{
		setCacheDirectory (QDir::homePath () + "/.leechcraft/core/cache");

		XmlSettingsManager::Instance ()->RegisterObject ("CacheSize",
				this, "handleCacheSize");
		handleCacheSize ();
	}

	QIODevice* NetworkDiskCache::prepare (const QNetworkCacheMetaData& metadata)
	{
		// Some quirks for disk cache, for example, don't cache https://
		// stuff as well as scripts.
		if (QString ("4.5.1") == qVersion () &&
				(metadata.url ().scheme () == "https" ||
				 metadata.url ().path ().endsWith ("js")))
			return 0;
		else
			return QNetworkDiskCache::prepare (metadata);
	}

	qint64 NetworkDiskCache::expire ()
	{
		collectGarbage ();
		if (PreviousSize_ >= 0)
			return PreviousSize_;
		else
			return maximumCacheSize () / 10 * 8;
	}

	void NetworkDiskCache::handleCacheSize ()
	{
		setMaximumCacheSize (XmlSettingsManager::Instance ()->
				property ("CacheSize").toInt () * 1048576);
		QTimer::singleShot (60000,
				this,
				SLOT (collectGarbage ()));
	}

	namespace
	{
		qint64 Collector (QString& cacheDirectory, qint64 goal)
		{
		    if (cacheDirectory.isEmpty ())
		        return 0;

		    QDir::Filters filters = QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot;
		    QDirIterator it (cacheDirectory, filters, QDirIterator::Subdirectories);

		    QMultiMap<QDateTime, QString> cacheItems;
		    qint64 totalSize = 0;
		    while (it.hasNext ())
		    {
		        QString path = it.next ();
		        QFileInfo info = it.fileInfo ();
		        QString fileName = info.fileName ();
		        if (fileName.endsWith(".cache") &&
		        		fileName.startsWith("cache_"))
		        {
		            cacheItems.insert(info.created (), path);
		            totalSize += info.size ();
		        }
		    }

		    QMultiMap<QDateTime, QString>::const_iterator i = cacheItems.constBegin();
		    while (i != cacheItems.constEnd())
		    {
		        if (totalSize < goal)
		            break;
		        QString name = i.value ();
		        QFile file (name);
		        qint64 size = file.size ();
		        file.remove ();
		        totalSize -= size;
		        ++i;
		    }

		    return totalSize;
		}
	};

	void NetworkDiskCache::collectGarbage ()
	{
		if (IsCollectingGarbage_)
			return;

		if (cacheDirectory ().isEmpty ())
			return;

		IsCollectingGarbage_ = true;

		QFutureWatcher<qint64> *watcher = new QFutureWatcher<qint64> (this);
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleCollectorFinished ()));

		QFuture<qint64> future = QtConcurrent::run (Collector,
				cacheDirectory (), maximumCacheSize () * 9 / 10);
		watcher->setFuture (future);
	}

	void NetworkDiskCache::handleCollectorFinished ()
	{
		QFutureWatcher<qint64> *watcher = dynamic_cast<QFutureWatcher<qint64>*> (sender ());

		PreviousSize_ = watcher->result ();

		IsCollectingGarbage_ = false;
	}
};

