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
#include "components/storage/sqlstoragebackend.h"
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

	void AddFeeds (UpdatesManager& updatesManager, const QList<AddFeedParams>& paramsList)
	{
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		QList<QString> duplicates;

		ids_t feedIDs;
		feedIDs.reserve (paramsList.size ());
		for (const auto& params : paramsList)
		{
			const auto& fixedUrl = QUrl::fromUserInput (params.URL_);
			const auto& url = fixedUrl.toString ();
			if (sb->FindFeed (url))
			{
				duplicates << url;
				continue;
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
			feedIDs << feed.FeedID_;
		}

		updatesManager.UpdateFeeds (feedIDs);

		if (!duplicates.isEmpty ())
		{
			qWarning () << "skipped duplicates:" << duplicates;

			const auto& msg = duplicates.size () > 1 ?
					QObject::tr ("%n already existing feed(s) were not added.", nullptr, duplicates.size ()) :
					QObject::tr ("Already existing feed %1 was not added.").arg (duplicates [0]);

			auto e = Util::MakeNotification (NotificationTitle, msg, Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}
	}

	void AddFeed (UpdatesManager& updatesManager, const AddFeedParams& params)
	{
		AddFeeds (updatesManager, { params });
	}
}
