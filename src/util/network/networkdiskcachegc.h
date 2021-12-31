/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <list>
#include <QObject>
#include <QMap>
#include <util/sll/util.h>

template<typename T>
class QFuture;

namespace LC::Util
{
	/** @brief Garbage collection for a set of network disk caches.
	 *
	 * This GC manager class aids having multiple network disk caches at
	 * the same path and running garbage collection periodically on them,
	 * but only once per each path.
	 *
	 * @ingroup NetworkUtil
	 */
	class NetworkDiskCacheGC : public QObject
	{
		using CacheSizeGetters_t = std::list<std::function<int ()>>;
		QMap<QString, CacheSizeGetters_t> Directories_;

		QMap<QString, qint64> LastSizes_;

		bool IsCollecting_ = false;

		NetworkDiskCacheGC ();
	public:
		NetworkDiskCacheGC (const NetworkDiskCacheGC&) = delete;
		NetworkDiskCacheGC& operator= (const NetworkDiskCacheGC&) = delete;

		/** @brief Returns a single global instance of the GC manager.
		 *
		 * @return The single global instance of the GC manager.
		 */
		static NetworkDiskCacheGC& Instance ();

		/** @brief Schedules calculation of the \em path total size.
		 *
		 * The calculation is performed asynchronously in a separate
		 * thread, and a future object is returned which can be used to
		 * be notified when the calculation finishes.
		 *
		 * @param[in] path The path which total size should be calculated
		 * @return The future object for the asynchronous path size
		 * calculation.
		 */
		QFuture<qint64> GetCurrentSize (const QString& path) const;

		/** @brief Registers the given cache \em path.
		 *
		 * Registers the given \em path to be collected periodically and
		 * returns a guard object that unregisters the path when it is
		 * destroyed.
		 *
		 * The \em path stops being collected as soon as the last guard
		 * object returned from this method is destroyed.
		 *
		 * This function also takes a size getter functor returning the
		 * desired cache size for the given \em path. If the same path is
		 * registered multiple times with size getters returning
		 * different values, the minimum one is used.
		 *
		 * @param[in] path The path to register for garbage collection.
		 * @param[in] sizeGetter The functor returning the desired total
		 * size of files under the \em path.
		 * @return A guard object unregistering the path when it is
		 * destroyed.
		 */
		Util::DefaultScopeGuard RegisterDirectory (const QString& path,
				const std::function<int ()>& sizeGetter);
	private:
		void UnregisterDirectory (const QString&, CacheSizeGetters_t::iterator);
		void HandleCollect ();
	};
}
