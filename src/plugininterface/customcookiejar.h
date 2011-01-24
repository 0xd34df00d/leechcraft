/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGININTERFACE_CUSTOMCOOKIEJAR_H
#define PLUGININTERFACE_CUSTOMCOOKIEJAR_H
#include <QNetworkCookieJar>
#include <QByteArray>
#include "piconfig.h"

namespace LeechCraft
{
	namespace Util
	{
		/** Customized cookie jar. Allows to filter tracking cookies, 
		 * filter duplicate cookies and has unlimited storage period.
		 */
		class PLUGININTERFACE_API CustomCookieJar : public QNetworkCookieJar
		{
			Q_OBJECT

			bool FilterTrackingCookies_;
		public:
			/** @brief Constructs the cookie jar.
			 *
			 * Filtering of tracking cookies is false by default, and
			 * cookies aren't restored.
			 *
			 * @param[in] parent The parent object.
			 */
			CustomCookieJar (QObject *parent = 0);
			
			/** Destructs the cookie jar.
			 */
			virtual ~CustomCookieJar ();

			/** Enables or disables filtering tracking cookies.
			 *
			 * @param[in] track Whether to filter tracking cookies.
			 */
			void SetFilterTrackingCookies (bool filter);

			/** Serializes the cookie jar contents into a QByteArray
			 * suitable for storage.
			 *
			 * @return The serialized cookies.
			 *
			 * @sa Load()
			 */
			QByteArray Save () const;

			/** Restores the cookies from the array previously obtained
			 * from Save().
			 *
			 * @param[in] data Serialized cookies.
			 * @sa Save()
			 */
			void Load (const QByteArray& data);

			/** Removes duplicate cookies.
			 */
			void CollectGarbage ();

			/** Returns cookies for the given url, filtering out
			 * duplicates.
			 *
			 * @param[in] url The url to return cookies for.
			 * @return The list of cookies, dup-free.
			 */
			QList<QNetworkCookie> cookiesForUrl (const QUrl& url) const;

			using QNetworkCookieJar::allCookies;
			using QNetworkCookieJar::setAllCookies;
		};
	};
};

#endif

