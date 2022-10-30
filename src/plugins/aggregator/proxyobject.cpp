/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "proxyobject.h"
#include <interfaces/core/icoreproxy.h>
#include "poolsmanager.h"
#include "storagebackendmanager.h"
#include "channelsmodel.h"
#include "itemslistmodel.h"
#include "dbutils.h"

namespace LC
{
namespace Aggregator
{
	namespace
	{
		void FixItemID (Item& item)
		{
			if (item.ItemID_)
				return;

			item.ItemID_ = PoolsManager::Instance ().GetPool (PTItem).GetID ();

			for (auto& enc : item.Enclosures_)
				enc.ItemID_ = item.ItemID_;
		}

		void FixChannelID (Channel& channel)
		{
			if (channel.ChannelID_)
				return;

			channel.ChannelID_ = PoolsManager::Instance ().GetPool (PTChannel).GetID ();
			for (const auto& item : channel.Items_)
			{
				item->ChannelID_ = channel.ChannelID_;
				FixItemID (*item);
			}
		}

		void FixFeedID (Feed& feed)
		{
			if (feed.FeedID_)
				return;

			feed.FeedID_ = PoolsManager::Instance ().GetPool (PTFeed).GetID ();

			for (const auto& channel : feed.Channels_)
			{
				channel->FeedID_ = feed.FeedID_;
				FixChannelID (*channel);
			}
		}
	}

	ProxyObject::ProxyObject (ChannelsModel *cm, QObject *parent)
	: QObject { parent }
	, ChannelsModel_ { cm }
	{
	}

	void ProxyObject::AddFeed (Feed feed)
	{
		FixFeedID (feed);
		StorageBackendManager::Instance ().MakeStorageBackendForThread ()->AddFeed (feed);
	}

	void ProxyObject::AddChannel (Channel channel)
	{
		FixChannelID (channel);
		StorageBackendManager::Instance ().MakeStorageBackendForThread ()->AddChannel (channel);
	}

	void ProxyObject::AddItem (Item item)
	{
		FixItemID (item);
		StorageBackendManager::Instance ().MakeStorageBackendForThread ()->AddItem (item);
	}

	QAbstractItemModel* ProxyObject::GetChannelsModel () const
	{
		return ChannelsModel_;
	}

	QVector<Channel> ProxyObject::GetAllChannels () const
	{
		QVector<Channel> result;
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& cs : GetAllChannels ())
			result << sb->GetChannel (cs.ChannelID_);
		return result;
	}

	Channel ProxyObject::GetChannel (IDType_t id) const
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		return sb->GetChannel (id);
	}

	int ProxyObject::CountUnreadItems (IDType_t channel) const
	{
		return StorageBackendManager::Instance ().MakeStorageBackendForThread ()->GetUnreadItemsCount (channel);
	}

	QVector<Item_ptr> ProxyObject::GetChannelItems (IDType_t channelId) const
	{
		return StorageBackendManager::Instance ().MakeStorageBackendForThread ()->GetFullItems (channelId);
	}

	std::optional<Item> ProxyObject::GetItem (IDType_t id) const
	{
		return StorageBackendManager::Instance ().MakeStorageBackendForThread ()->GetItem (id);
	}

	void ProxyObject::SetItemRead (IDType_t id, bool read) const
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		auto item = sb->GetItem (id);
		if (!item)
			return;

		item->Unread_ = !read;
		sb->UpdateItem (*item);
	}

	QAbstractItemModel* ProxyObject::CreateItemsModel () const
	{
		return new ItemsListModel { GetProxyHolder ()->GetIconThemeManager () };
	}
}
}
