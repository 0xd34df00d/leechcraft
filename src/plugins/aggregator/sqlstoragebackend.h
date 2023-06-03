/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <util/sll/util.h>
#include <util/db/oral/oralfwd.h>
#include "storagebackend.h"

namespace LC::Aggregator
{
	class SQLStorageBackend : public StorageBackend
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
		const Type Type_;

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
		SQLStorageBackend (Type, const QString&);
		~SQLStorageBackend ();

		void Prepare () override;

		ids_t GetFeedsIDs () const override;
		Feed GetFeed (IDType_t) const override;
		std::optional<IDType_t> FindFeed (const QString&) const override;
		std::optional<Feed::FeedSettings> GetFeedSettings (IDType_t) const override;
		void SetFeedSettings (const Feed::FeedSettings&) override;
		std::optional<QStringList> GetFeedTags (IDType_t) const override;
		void SetFeedTags (IDType_t, const QStringList&) override;
		void SetFeedURL (IDType_t, const QString&) override;

		channels_shorts_t GetChannels (IDType_t) const override;
		Channel GetChannel (IDType_t) const override;
		std::optional<IDType_t> FindChannel (const QString& , const QString&, IDType_t) const override;
		void TrimChannel (IDType_t, int, int) override;
		std::optional<QImage> GetChannelPixmap (IDType_t) const override;
		void SetChannelPixmap (IDType_t, const std::optional<QImage>&) override;
		void SetChannelFavicon (IDType_t, const std::optional<QImage>&) override;
		void SetChannelTags (IDType_t, const QStringList&) override;
		void SetChannelDisplayTitle (IDType_t, const QString&) override;
		void SetChannelTitle (IDType_t, const QString&) override;
		void SetChannelLink (IDType_t, const QString&) override;

		items_shorts_t GetItems (IDType_t) const override;
		int GetUnreadItemsCount (IDType_t) const override;
		int GetTotalItemsCount (IDType_t) const override;
		std::optional<Item> GetItem (IDType_t) const override;
		std::optional<IDType_t> FindItem (const QString&, const QString&, IDType_t) const override;
		std::optional<IDType_t> FindItemByLink (const QString&, IDType_t) const override;
		std::optional<IDType_t> FindItemByTitle (const QString&, IDType_t) const override;

		void AddFeed (const Feed&) override;
		void UpdateItem (const Item&) override;
		void SetItemUnread (IDType_t, IDType_t, bool) override;
		void AddChannel (const Channel&) override;
		void AddItem (const Item&) override;
		void RemoveItems (const QSet<IDType_t>&) override;
		void RemoveChannel (IDType_t) override;
		void RemoveFeed (IDType_t) override;
		bool UpdateFeedsStorage (int) override;
		bool UpdateChannelsStorage (int) override;
		bool UpdateItemsStorage (int) override;
		void ToggleChannelUnread (IDType_t, bool) override;

		QList<ITagsManager::tag_id> GetItemTags (IDType_t) override;
		void SetItemTags (IDType_t, const QList<ITagsManager::tag_id>&) override;
		QList<IDType_t> GetItemsForTag (const ITagsManager::tag_id&) override;

		IDType_t GetHighestID (const PoolType&) const override;
	private:
		void WriteEnclosures (const QList<Enclosure>&);
		void GetEnclosures (IDType_t, QList<Enclosure>&) const;
		void WriteMRSSEntries (const QList<MRSSEntry>&);
		void GetMRSSEntries (IDType_t, QList<MRSSEntry>&) const;
		IDType_t GetHighestID (const QByteArray&, const QByteArray&) const;
	};
}
