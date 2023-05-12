/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/aggregator/iproxyobject.h"

namespace LC
{
namespace Aggregator
{
	class ChannelsModel;

	class ProxyObject : public QObject
					  , public IProxyObject
	{
		Q_OBJECT
		Q_INTERFACES (LC::Aggregator::IProxyObject)

		ChannelsModel * const ChannelsModel_;
	public:
		explicit ProxyObject (ChannelsModel*, QObject* = nullptr);

		/** @brief Adds the given feed to the storage.
		 *
		 * This method adds the given feed, all its channels and items
		 * to the storage. The storage isn't checked whether it already
		 * contains this feed.
		 *
		 * If the feed's FeedID_ member is set to a non-zero value, that
		 * value is used as the feed's ID. In this case it's your duty
		 * to make sure that this is a correct ID. Otherwise, if
		 * Feed::FeedID_ is 0, a suitable ID is generated and used, and
		 * the feed is updated accordingly. This way one could know the
		 * assigned ID.
		 *
		 * This function may throw.
		 *
		 * @param[in,out] feed The feed to add.
		 *
		 * @exception std::exception
		 */
		void AddFeed (Feed feed) override;

		/** @brief Adds the given channel to the storage.
		 *
		 * This method adds the given channel and all its items to the
		 * storage. The storage isn't checked whether it already
		 * contains this channel.
		 *
		 * It's your duty to ensure that the feed mentioned as the
		 * channel's parent feed actually exists.
		 *
		 * The same applies to the channel's ChannelID_ as for FeedID_
		 * in AddFeed().
		 *
		 * This function may throw.
		 *
		 * @param[in,out] channel The channel to add.
		 *
		 * @exception std::exception
		 */
		void AddChannel (Channel channel) override;

		/** @brief Adds the given item to the storage.
		 *
		 * This method adds the given item to the storage. The storage
		 * isn't checked whether it already contains this item.
		 *
		 * It's your duty to ensure that the channel mentioned as the
		 * item's parent channel actually exists.
		 *
		 * The same applies to the item's ItemID_ as for FeedID_ in
		 * AddFeed().
		 *
		 * This function may throw.
		 *
		 * @param[in,out] item The item to add.
		 *
		 * @exception std::exception
		 */
		void AddItem (Item item) override;

		QAbstractItemModel* GetChannelsModel () const override;
		Channel GetChannel (IDType_t) const override;

		std::optional<Item> GetItem (IDType_t) const override;
		void SetItemRead (IDType_t, bool) const override;

		std::unique_ptr<IItemsModel> CreateItemsModel () const override;
	};
}
}
