/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkDiskCache>
#include <QRecursiveMutex>
#include <QHash>
#include <util/sll/util.h>
#include "networkconfig.h"

namespace LC::Util
{
	/** @brief A thread-safe garbage-collected network disk cache.
	 *
	 * This class is thread-safe unlike the original QNetworkDiskCache,
	 * thus it can be used from multiple threads simultaneously.
	 *
	 * Also, old cache data is automatically removed from the cache in a
	 * background thread without blocking. The garbage collection can be
	 * also triggered manually via the collectGarbage() slot.
	 *
	 * The garbage is collected until cache takes 90% of its maximum size.
	 *
	 * @ingroup NetworkUtil
	 */
	class UTIL_NETWORK_API NetworkDiskCache : public QNetworkDiskCache
	{
		Q_OBJECT

		qint64 CurrentSize_ = -1;

		mutable QRecursiveMutex InsertRemoveMutex_;

		QHash<QIODevice*, QUrl> PendingDev2Url_;
		QHash<QUrl, QList<QIODevice*>> PendingUrl2Devs_;

		const Util::DefaultScopeGuard GcGuard_;
	public:
		/** @brief Constructs the new disk cache.
		 *
		 * The cache uses a subdirectory \em subpath in the \em network
		 * directory of the user cache location.
		 *
		 * @param[in] subpath The subpath in cache user location.
		 * @param[in] parent The parent object of this cache.
		 *
		 * @sa GetUserDir(), UserDir::Cache.
		 */
		explicit NetworkDiskCache (const QString& subpath, QObject *parent = nullptr);

		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		qint64 cacheSize () const override;

		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		QIODevice* data (const QUrl& url) override;

		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		void insert (QIODevice *device) override;

		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		QNetworkCacheMetaData metaData (const QUrl& url) override;

		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		QIODevice* prepare (const QNetworkCacheMetaData&) override;

		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		bool remove (const QUrl& url) override;

		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		void updateMetaData (const QNetworkCacheMetaData& metaData) override;
	protected:
		/** @brief Reimplemented from QNetworkDiskCache.
		 */
		qint64 expire () override;
	};
}

