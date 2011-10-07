/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "proxyobject.h"
#include <boost/foreach.hpp>
#include "core.h"

namespace LeechCraft
{
namespace Aggregator
{
	ProxyObject::ProxyObject (QObject *parent)
	: QObject (parent)
	{
	}

	namespace
	{
		void FixItemID (Item_ptr item)
		{
			if (item->ItemID_)
				return;

			item->ItemID_ = Core::Instance ().GetPool (PTItem).GetID ();

			BOOST_FOREACH (Enclosure& enc, item->Enclosures_)
				enc.ItemID_ = item->ItemID_;
		}

		void FixChannelID (Channel_ptr channel)
		{
			if (channel->ChannelID_)
				return;

			channel->ChannelID_ = Core::Instance ().GetPool (PTChannel).GetID ();
			Q_FOREACH (Item_ptr item, channel->Items_)
			{
				item->ChannelID_ = channel->ChannelID_;

				FixItemID (item);
			}
		}

		void FixFeedID (Feed_ptr feed)
		{
			if (feed->FeedID_)
				return;

			feed->FeedID_ = Core::Instance ().GetPool (PTFeed).GetID ();

			Q_FOREACH (Channel_ptr channel, feed->Channels_)
			{
				channel->FeedID_ = feed->FeedID_;

				FixChannelID (channel);
			}
		}
	}

	void ProxyObject::AddFeed (Feed_ptr feed)
	{
		FixFeedID (feed);

		Core::Instance ().GetStorageBackend ()->AddFeed (feed);
	}

	void ProxyObject::AddChannel (Channel_ptr channel)
	{
		FixChannelID (channel);

		Core::Instance ().GetStorageBackend ()->AddChannel (channel);
	}

	void ProxyObject::AddItem (Item_ptr item)
	{
		FixItemID (item);

		Core::Instance ().GetStorageBackend ()->AddItem (item);
	}
}
}
