/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkdiskcache.h"
#include <QtDebug>
#include <QDir>
#include <QFuture>
#include <QMutexLocker>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include "networkdiskcachegc.h"

namespace LC
{
namespace Util
{
	namespace
	{
		QString GetCacheDir (const QString& subpath)
		{
			return GetUserDir (UserDir::Cache, "network/" + subpath).absolutePath ();
		}
	}

	NetworkDiskCache::NetworkDiskCache (const QString& subpath, QObject *parent)
	: QNetworkDiskCache (parent)
	, CurrentSize_ (-1)
	, InsertRemoveMutex_ (QMutex::Recursive)
	, GcGuard_ (NetworkDiskCacheGC::Instance ().RegisterDirectory (GetCacheDir (subpath),
			[this] { return maximumCacheSize (); }))
	{
		setCacheDirectory (GetCacheDir (subpath));
	}

	qint64 NetworkDiskCache::cacheSize () const
	{
		return CurrentSize_;
	}

	QIODevice* NetworkDiskCache::data (const QUrl& url)
	{
		QMutexLocker lock (&InsertRemoveMutex_);
		return QNetworkDiskCache::data (url);
	}

	void NetworkDiskCache::insert (QIODevice *device)
	{
		QMutexLocker lock (&InsertRemoveMutex_);
		if (!PendingDev2Url_.contains (device))
		{
			qWarning () << Q_FUNC_INFO
					<< "stall device detected";
			return;
		}

		PendingUrl2Devs_ [PendingDev2Url_.take (device)].removeAll (device);

		CurrentSize_ += device->size ();
		QNetworkDiskCache::insert (device);
	}

	QNetworkCacheMetaData NetworkDiskCache::metaData (const QUrl& url)
	{
		QMutexLocker lock (&InsertRemoveMutex_);
		return QNetworkDiskCache::metaData (url);
	}

	QIODevice* NetworkDiskCache::prepare (const QNetworkCacheMetaData& metadata)
	{
		QMutexLocker lock (&InsertRemoveMutex_);
		const auto dev = QNetworkDiskCache::prepare (metadata);
		PendingDev2Url_ [dev] = metadata.url ();
		PendingUrl2Devs_ [metadata.url ()] << dev;
		return dev;
	}

	bool NetworkDiskCache::remove (const QUrl& url)
	{
		QMutexLocker lock (&InsertRemoveMutex_);
		for (const auto dev : PendingUrl2Devs_.take (url))
			PendingDev2Url_.remove (dev);
		return QNetworkDiskCache::remove (url);
	}

	void NetworkDiskCache::updateMetaData (const QNetworkCacheMetaData& metaData)
	{
		QMutexLocker lock (&InsertRemoveMutex_);
		QNetworkDiskCache::updateMetaData (metaData);
	}

	qint64 NetworkDiskCache::expire ()
	{
		if (CurrentSize_ < 0)
		{
			const auto& dir = cacheDirectory ();
			Util::Sequence (this, NetworkDiskCacheGC::Instance ().GetCurrentSize (dir)) >>
					[this] (qint64 res) { CurrentSize_ = res; };

			return maximumCacheSize () * 8 / 10;
		}

		return CurrentSize_;
	}
}
}
