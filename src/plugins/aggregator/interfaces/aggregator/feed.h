/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_INTERFACES_AGGREGATOR_FEED_H
#define PLUGINS_AGGREGATOR_INTERFACES_AGGREGATOR_FEED_H
#include <memory>
#include <vector>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMetaType>
#include "channel.h"
#include "common.h"

namespace LC
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
			/** @brief ID of the corresponding feed.
			 */
			IDType_t FeedID_ = IDNotFound;

			/** @brief Update timeout for the feed.
			 *
			 * How often the feed should be checked for updates. The default
			 * value is 0.
			 */
			int UpdateTimeout_ = 0;

			/** @brief Max number of stored items.
			 *
			 * How much items should be stored in the feed's channels. The
			 * default value is 0.
			 */
			int NumItems_ = 0;

			/** @brief Max item's age.
			 *
			 * How old could be items in the feed. The default value is 0.
			 */
			int ItemAge_ = 0;

			/** @brief Automatically download enclosures.
			 *
			 * Whether Aggregator should automatically emit an
			 * entity for each enclosure in each news item it
			 * fetches.
			 */
			bool AutoDownloadEnclosures_ = false;
		};

		IDType_t FeedID_;
		QString URL_;
		QDateTime LastUpdate_;
		channels_container_t Channels_;

		Feed ();
		Feed (IDType_t id);
		Feed (IDType_t, const QString&, const QDateTime&);
	};

	typedef std::shared_ptr<Feed::FeedSettings> FeedSettings_ptr;
	typedef std::shared_ptr<Feed> Feed_ptr;
	typedef std::vector<Feed_ptr> feeds_container_t;
	typedef std::vector<QString> feeds_urls_t;

	bool operator< (const Feed& f1, const Feed& f2);
}
}

Q_DECLARE_METATYPE (LC::Aggregator::Feed)

#endif
