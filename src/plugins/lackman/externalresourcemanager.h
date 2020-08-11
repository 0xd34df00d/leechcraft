/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_EXTERNALRESOURCEMANAGER_H
#define PLUGINS_LACKMAN_EXTERNALRESOURCEMANAGER_H
#include <QObject>
#include <QUrl>
#include <QDir>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

namespace LC
{
namespace LackMan
{
	/** Manages external resources like images, icons,
		* screenshots and even package files.
		*
		* Resource manager keeps fetched resource in its own cache
		* and is generally asynchronous. Thus, if a requested
		* resource isn't available and it isn't fetched yet,
		* the manager would schedule resource fetching and emit the
		* resourceFetched() signal once the resource is fetched.
		*/
	class ExternalResourceManager : public QObject
	{
		Q_OBJECT

		QSet<QUrl> PendingResources_;

		QDir ResourcesDir_;
	public:
		ExternalResourceManager (QObject* = 0);

		/** @brief Fetches resource at \em url.
		 *
		 * Starts fetching the resource identified by \em url. After the
		 * resource is fetched, the resourceFetched() signal is emitted.
		 *
		 * If the resource identified by the \em url is already fetched,
		 * this function does nothing.
		 *
		 * @param[in] url URL of the resource to fetch.
		 *
		 * @sa resourceFetched()
		 */
		void GetResourceData (const QUrl& url);

		/** @brief Returns the path of the resource at a given
		 * url.
		 *
		 * This function returns the proper path even if the
		 * resource hasn't been fetched yet. In this case, there
		 * would be just no file at the returned path. The file
		 * at the returned path is guaranteed to exist and be
		 * valid only after resourceFetched() signal has been
		 * emitted for this url or if GetResourceData() returns
		 * proper data.
		 *
		 * @param[in] url URL of the resource to get the path
		 * for.
		 * @return The local path of the fetched copy of the
		 * url.
		 */
		QString GetResourcePath (const QUrl& url) const;

		/** Clears all fetched resources.
			*/
		void ClearCaches ();

		/** @brief Clears fetched resource identified by url.
		 *
		 * If the resource identified by url isn't fetched, this
		 * function does nothing.
		 *
		 * @param[in] url URL of the resource to remove from
		 * cache.
		 */
		void ClearCachedResource (const QUrl& url);
	signals:
		/** @brief Emitted once the resource identified by url
		 * is fetched.
		 *
		 * After this signal GetResourceData() function is
		 * guaranteed to return valid and actual data for the
		 * resource identified by the emitted url.
		 *
		 * @param[out] url URL of the resource just fetched.
		 *
		 * @sa GetResourceData()
		 */
		void resourceFetched (const QUrl& url);
	};
}
}

#endif
