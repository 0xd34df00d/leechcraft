/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dbutils.h"
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include "components/storage/storagebackend.h"
#include "components/storage/storagebackendmanager.h"
#include "updatesmanager.h"

namespace LC::Aggregator
{
	channels_shorts_t GetAllChannels ()
	{
		channels_shorts_t result;

		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto id : sb->GetFeedsIDs ())
		{
			auto feedChannels = sb->GetChannels (id);
			std::move (feedChannels.begin (), feedChannels.end (), std::back_inserter (result));
		}

		return result;
	}

	void AddFeed (const AddFeedParams& params)
	{
		auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		const auto& fixedUrl = QUrl::fromUserInput (params.URL_);
		const auto& url = fixedUrl.toString ();
		if (sb->FindFeed (url))
		{
			auto e = Util::MakeNotification (NotificationTitle,
					QObject::tr ("The feed %1 is already added")
						.arg (url),
					Priority::Critical);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
			return;
		}

		Feed feed;
		feed.URL_ = url;
		sb->AddFeed (feed);
		sb->SetFeedTags (feed.FeedID_, GetProxyHolder ()->GetTagsManager ()->GetIDs (params.Tags_));

		if (params.FeedSettings_)
		{
			auto fs = *params.FeedSettings_;
			fs.FeedID_ = feed.FeedID_;
			sb->SetFeedSettings (fs);
		}

		params.UpdatesManager_.UpdateFeed (feed.FeedID_);
	}
}
