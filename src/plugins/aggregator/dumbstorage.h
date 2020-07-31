/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "storagebackend.h"

namespace LC
{
namespace Aggregator
{
	class DumbStorage : public StorageBackend
	{
	public:
		void Prepare () override {}
		ids_t GetFeedsIDs () const override { return {}; }
		Feed GetFeed (IDType_t) const override { return {}; }
		std::optional<IDType_t> FindFeed (const QString&) const override { return {}; }
		std::optional<Feed::FeedSettings> GetFeedSettings (IDType_t) const override { return {}; }
		void SetFeedSettings (const Feed::FeedSettings&) override {}
		std::optional<QStringList> GetFeedTags (IDType_t) const override { return {}; }
		void SetFeedTags (IDType_t, const QStringList&) override {}
		void SetFeedURL (IDType_t, const QString&) override {}
		channels_shorts_t GetChannels (IDType_t) const override { return {}; }
		Channel GetChannel (IDType_t) const override { return {}; }
		std::optional<IDType_t> FindChannel (const QString&, const QString&, IDType_t) const override { return {}; }
		void TrimChannel (IDType_t, int, int) override {}
		std::optional<QImage> GetChannelPixmap (IDType_t) const override { return {}; }
		void SetChannelPixmap (IDType_t, const std::optional<QImage>&) override {}
		void SetChannelFavicon (IDType_t, const std::optional<QImage>&) override {}
		void SetChannelTags (IDType_t, const QStringList&) override {}
		void SetChannelDisplayTitle (IDType_t, const QString&) override {}
		void SetChannelTitle (IDType_t, const QString&) override {}
		void SetChannelLink (IDType_t, const QString&) override {}
		items_shorts_t GetItems (IDType_t) const override { return {}; }
		int GetUnreadItemsCount (IDType_t) const override { return {}; }
		int GetTotalItemsCount (IDType_t) const override { return {}; }
		std::optional<Item> GetItem (IDType_t) const override { return {}; }
		std::optional<IDType_t> FindItem (const QString&, const QString&, IDType_t) const override { return {}; }
		std::optional<IDType_t> FindItemByTitle (const QString&, IDType_t) const override { return {}; }
		std::optional<IDType_t> FindItemByLink (const QString&, IDType_t) const override { return {}; }
		items_container_t GetFullItems (IDType_t) const override { return {}; }
		void AddFeed (const Feed&) override {}
		void AddChannel (const Channel&) override {}
		void AddItem (const Item&) override {}
		void UpdateItem (const Item&) override {}
		void SetItemUnread (IDType_t, bool) override {}
		void RemoveItems (const QSet<IDType_t>&) override {}
		void RemoveChannel (IDType_t) override {}
		void RemoveFeed (IDType_t) override {}
		bool UpdateFeedsStorage (int) override { return {}; }
		bool UpdateChannelsStorage (int) override { return {}; }
		bool UpdateItemsStorage (int) override { return {}; }
		void ToggleChannelUnread (IDType_t, bool) override {}
		QList<ITagsManager::tag_id> GetItemTags (IDType_t) override { return {}; }
		void SetItemTags (IDType_t, const QList<ITagsManager::tag_id>&) override {}
		QList<IDType_t> GetItemsForTag (const ITagsManager::tag_id&) override { return {}; }
		IDType_t GetHighestID (const PoolType&) const override { return {}; }
	};
}
}
