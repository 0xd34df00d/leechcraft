/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_LACKMAN_EXTERNALRESOURCEMANAGER_H
#define PLUGINS_LACKMAN_EXTERNALRESOURCEMANAGER_H
#include <boost/optional.hpp>
#include <QObject>
#include <QUrl>
#include <QDir>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

namespace LeechCraft
{
	namespace Plugins
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

				struct PendingResource
				{
					QUrl URL_;
				};
				QMap<int, PendingResource> PendingResources_;

				QDir ResourcesDir_;
			public:
				ExternalResourceManager (QObject* = 0);

				/** @brief Fetches/returns (if cached) resource at url.
				 *
				 * If the resource identified by the given url is
				 * already fetched, this function just returns its
				 * contents. Otherwise, it starts fetching the resource.
				 * After the resource is fetched, the resourceFetched()
				 * signal is emitted.
				 *
				 * @param[in] url URL of the resource to fetch/return.
				 * @return The resource data (if fetched already) or
				 * nothing (if not).
				 */
				boost::optional<QByteArray> GetResourceData (const QUrl& url);

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
			private slots:
				void handleResourceFinished (int);
				void handleResourceRemoved (int);
				void handleResourceError (int, IDownload::Error);
			signals:
				/** @brief Emitted once the resource identified by url
				 * is fetched.
				 *
				 * After this signal GetResourceData() function is
				 * guaranteed to return valid and actual data for the
				 * resource identified by the emitted url.
				 *
				 * @param[out] url URL of the resource just fetched.
				 */
				void resourceFetched (const QUrl& url);

				void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
			};
		}
	}
}

#endif
