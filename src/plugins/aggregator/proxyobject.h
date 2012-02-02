/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_PROXYOBJECT_H
#define PLUGINS_AGGREGATOR_PROXYOBJECT_H
#include <QObject>
#include "interfaces/aggregator/iproxyobject.h"

namespace LeechCraft
{
namespace Aggregator
{
	class ProxyObject : public QObject
					  , public IProxyObject
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Aggregator::IProxyObject)
	public:
		ProxyObject (QObject* = 0);

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
		void AddFeed (Feed_ptr feed);

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
		void AddChannel (Channel_ptr channel);

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
		void AddItem (Item_ptr item);

		QList<Channel_ptr> GetAllChannels () const;
		int CountUnreadItems (IDType_t) const;
		QList<Item_ptr> GetChannelItems (IDType_t) const;
	};
}
}

#endif
