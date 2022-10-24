/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include "common.h"

class QAbstractItemModel;

namespace LC
{
namespace Aggregator
{
	struct Item;
	struct Channel;
	struct Feed;

	typedef std::shared_ptr<Item> Item_ptr;
	typedef std::shared_ptr<Channel> Channel_ptr;
	typedef std::shared_ptr<Feed> Feed_ptr;

	class IProxyObject
	{
	public:
		virtual ~IProxyObject () {}

		virtual void AddFeed (Feed) = 0;
		virtual void AddChannel (Channel) = 0;
		virtual void AddItem (Item) = 0;

		virtual QAbstractItemModel* GetChannelsModel () const = 0;
		virtual QVector<Channel> GetAllChannels () const = 0;
		virtual Channel GetChannel (IDType_t) const = 0;
		virtual int CountUnreadItems (IDType_t) const = 0;
		virtual QVector<Item_ptr> GetChannelItems (IDType_t) const = 0;

		virtual std::optional<Item> GetItem (IDType_t) const = 0;
		virtual void SetItemRead (IDType_t, bool) const = 0;

		virtual QAbstractItemModel* CreateItemsModel () const = 0;
	};

	typedef std::shared_ptr<IProxyObject> IProxyObject_ptr;
}
}

Q_DECLARE_INTERFACE (LC::Aggregator::IProxyObject,
		"org.Deviant.LeechCraft.Aggregator.IProxyObject/1.0")
