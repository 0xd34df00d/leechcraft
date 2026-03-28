/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSqlDatabase>
#include <util/sll/util.h>
#include <util/db/oral/oralfwd.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "feed.h"

namespace LC::Aggregator
{
	struct UnreadDelta
	{
			int delta;
			auto operator<=> (const UnreadDelta&) const = default;
	};

	struct UnreadTotal
	{
			qsizetype total;
			auto operator<=> (const UnreadTotal&) const = default;
	};

	struct UnreadChange : std::variant<UnreadDelta, UnreadTotal>
	{
			using variant::variant;
			auto operator<=> (const UnreadChange&) const = default;
	};

	class SQLStorageBackend : public QObject
	{
		Q_OBJECT

		Util::DefaultScopeGuard DBRemover_;
		QSqlDatabase DB_;
	public:
		struct FeedR;
		struct FeedSettingsR;
		struct ChannelR;
		struct ItemR;

		struct EnclosureR;

		struct MRSSEntryR;
		struct MRSSThumbnailR;
		struct MRSSCreditR;
		struct MRSSCommentR;
		struct MRSSPeerLinkR;
		struct MRSSSceneR;

		struct Item2TagsR;

		struct Feed2TagsR;
	private:
		Util::oral::ObjectInfo_ptr<FeedR> Feeds_;
		Util::oral::ObjectInfo_ptr<FeedSettingsR> FeedsSettings_;
		Util::oral::ObjectInfo_ptr<ChannelR> Channels_;
		Util::oral::ObjectInfo_ptr<ItemR> Items_;
		Util::oral::ObjectInfo_ptr<EnclosureR> Enclosures_;
		Util::oral::ObjectInfo_ptr<MRSSThumbnailR> MRSSThumbnails_;
		Util::oral::ObjectInfo_ptr<MRSSCreditR> MRSSCredits_;
		Util::oral::ObjectInfo_ptr<MRSSCommentR> MRSSComments_;
		Util::oral::ObjectInfo_ptr<MRSSPeerLinkR> MRSSPeerLinks_;
		Util::oral::ObjectInfo_ptr<MRSSSceneR> MRSSScenes_;
		Util::oral::ObjectInfo_ptr<MRSSEntryR> MRSSEntries_;
		Util::oral::ObjectInfo_ptr<Item2TagsR> Items2Tags_;
		Util::oral::ObjectInfo_ptr<Feed2TagsR> Feeds2Tags_;
	public:
		explicit SQLStorageBackend (const QString& = {});
		~SQLStorageBackend ();

		struct FeedNotFoundError {};
		struct ChannelNotFoundError {};
		struct ItemNotFoundError {};

		enum class ReadStatus
		{
			All,
			Read,
			Unread,
		};

		ids_t GetFeedsIDs () const;
		Feed GetFeed (IDType_t) const;
		std::optional<IDType_t> FindFeed (const QString&) const;
		std::optional<Feed::FeedSettings> GetFeedSettings (IDType_t) const;
		void SetFeedSettings (const Feed::FeedSettings&);
		std::optional<QStringList> GetFeedTags (IDType_t) const;
		void SetFeedTags (IDType_t, const QStringList&);
		void SetFeedURL (IDType_t, const QString&);

		channels_shorts_t GetChannels (IDType_t) const;
		Channel GetChannel (IDType_t) const;
		std::optional<IDType_t> FindChannel (const QString& , const QString&, IDType_t) const;
		void TrimChannel (IDType_t, int, int);
		std::optional<QImage> GetChannelPixmap (IDType_t) const;
		void SetChannelPixmap (IDType_t, const std::optional<QImage>&);
		void SetChannelFavicon (IDType_t, const std::optional<QImage>&);
		void SetChannelTags (IDType_t, const QStringList&);
		void SetChannelDisplayTitle (IDType_t, const QString&);
		void SetChannelTitle (IDType_t, const QString&);
		void SetChannelLink (IDType_t, const QString&);

		items_shorts_t GetItems (IDType_t) const;
		QSet<QString> GetItemsCategories (IDType_t, ReadStatus) const;
		int GetUnreadItemsCount (IDType_t) const;
		int GetTotalItemsCount (IDType_t) const;
		std::optional<Item> GetItem (IDType_t) const;
		std::optional<IDType_t> FindItem (const QString&, const QString&, IDType_t) const;
		std::optional<IDType_t> FindItemByLink (const QString&, IDType_t) const;
		std::optional<IDType_t> FindItemByTitle (const QString&, IDType_t) const;

		void AddFeed (const Feed&);
		void UpdateItem (const Item&);
		void SetItemUnread (IDType_t, IDType_t, bool);
		void AddChannel (const Channel&);
		void AddItem (const Item&);
		void RemoveItems (const QSet<IDType_t>&);
		void RemoveChannel (IDType_t);
		void RemoveFeed (IDType_t);
		void ToggleChannelUnread (IDType_t, bool);

		QList<ITagsManager::tag_id> GetItemTags (IDType_t);
		void SetItemTags (IDType_t, const QList<ITagsManager::tag_id>&);
		QList<IDType_t> GetItemsForTag (const ITagsManager::tag_id&);

		IDType_t GetHighestID (const PoolType&) const;
	private:
		void WriteItem (const Item&);
		void WriteEnclosures (const QList<Enclosure>&);
		void GetEnclosures (IDType_t, QList<Enclosure>&) const;
		void WriteMRSSEntries (const QList<MRSSEntry>&);
		void GetMRSSEntries (IDType_t, QList<MRSSEntry>&) const;
		IDType_t GetHighestID (const QByteArray&, const QByteArray&) const;
	signals:
		void channelAdded (const Channel& channel) const;
		void channelUnreadCountUpdated (IDType_t channelId, const UnreadChange& unreadChange) const;
		void channelDataUpdated (const Channel&) const;
		void itemReadStatusUpdated (IDType_t channelId, IDType_t itemId, bool unread) const;
		void itemDataUpdated (const Item& item) const;
		void itemsRemoved (const QSet<IDType_t>& items) const;
		void channelRemoved (IDType_t channelId);
		void feedRemoved (IDType_t feedId);

		void hookItemLoad (LC::IHookProxy_ptr proxy, Item *item) const;
		void hookItemAdded (LC::IHookProxy_ptr proxy, const Item& item) const;
	};

	using SQLStorageBackend_ptr = std::shared_ptr<SQLStorageBackend>;
}
