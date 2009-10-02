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

#ifndef PLUGINS_AGGREGATOR_FEED_H
#define PLUGINS_AGGREGATOR_FEED_H
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "channel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct Feed
			{
				/** @brief Contains settings for a feed.
				 * 
				 * If a feed overrides default settings like update timeout, number of
				 * stored items or max items age, this structure is used to store these
				 * settings in the storage backend.
				 *
				 * The feed can override only some of the settings, in this case others
				 * have setting-specific default value.
				 */
				struct FeedSettings
				{
					/** @brief Returns a default-constructed feed.
					 *
					 * A default-constructed feed has all the settings set to default
					 * values - so, no settings are overridden.
					 *
					 * @param[in] ut Update interval.
					 * @param[in] ni Number of items.
					 * @param[in] ia Max age.
					 * @param[in] ada Automatically download enclosures.
					 */
					FeedSettings (int ut = 0, int ni = 0, int ia = 0, bool ada = false);

					/** @brief Update timeout for the feed.
					 *
					 * How often the feed should be checked for updates. The default
					 * value is 0.
					 */
					int UpdateTimeout_;

					/** @brief Max number of stored items.
					 *
					 * How much items should be stored in the feed's channels. The
					 * default value is 0.
					 */
					int NumItems_;

					/** @brief Max item's age.
					 *
					 * How old could be items in the feed. The default value is 0.
					 */
					int ItemAge_;

					/** @brief Automatically download enclosures.
					 *
					 * Whether Aggregator should automatically emit a
					 * gotEntity for each enclosure in each news item it
					 * fetches.
					 */
					bool AutoDownloadEnclosures_;
				};

				QString URL_;
				QDateTime LastUpdate_;
				channels_container_t Channels_;

				Feed ();
				Feed (const Feed&);
				Feed& operator= (const Feed&);
			};

			typedef boost::shared_ptr<Feed> Feed_ptr;
			typedef std::vector<Feed_ptr> feeds_container_t;
			typedef std::vector<QString> feeds_urls_t;

			bool operator< (const Feed& f1, const Feed& f2);
			QDataStream& operator<< (QDataStream&, const Feed&);
			QDataStream& operator>> (QDataStream&, Feed&);
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Aggregator::Feed);

#endif

