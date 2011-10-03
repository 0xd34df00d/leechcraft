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
#include "core.h"

namespace LeechCraft
{
namespace Aggregator
{
	ProxyObject::ProxyObject (QObject *parent)
	: QObject (parent)
	{
	}

	void ProxyObject::AddFeed (Feed_ptr feed)
	{
		if (!feed->FeedID_)
			feed->FeedID_ = Core::Instance ().GetPool (PTFeed).GetID ();

		Core::Instance ().GetStorageBackend ()->AddFeed (feed);
	}

	void ProxyObject::AddChannel (Channel_ptr channel)
	{
		if (!channel->ChannelID_)
			channel->ChannelID_ = Core::Instance ().GetPool (PTChannel).GetID ();

		Core::Instance ().GetStorageBackend ()->AddChannel (channel);
	}

	void ProxyObject::AddItem (Item_ptr item)
	{
		if (!item->ItemID_)
			item->ItemID_ = Core::Instance ().GetPool (PTItem).GetID ();

		Core::Instance ().GetStorageBackend ()->AddItem (item);
	}
}
}
