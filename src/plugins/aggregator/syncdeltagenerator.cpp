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

#include "syncdeltagenerator.h"
#include "feed.h"

namespace LeechCraft
{
namespace Aggregator
{
	Sync::Payloads_t SyncDeltaGenerator::GetFeedAdded (const Feed& feed)
	{
		Sync::Payloads_t result;
		
		Sync::Payload fp;
		{
			QDataStream ostr (&fp.Data_, QIODevice::WriteOnly);
			ostr << static_cast<quint16> (PTFeedAdded);
			ostr << feed.URL_;
		}
		result << fp;
		
		Q_FOREACH (const Channel_ptr& c, feed.Channels_)
			result += GetChanAdded (c);

		return result;
	}

	Sync::Payloads_t SyncDeltaGenerator::GetChanAdded (Channel_ptr channel)
	{
		Sync::Payloads_t result;

		Sync::Payload item;
		
		{
			QDataStream ostr (&item.Data_, QIODevice::WriteOnly);
			ostr << static_cast<quint16> (PTChanAdded);
			ostr << channel;
		}
		
		result << item;

		return result;
	}

	Sync::Payloads_t SyncDeltaGenerator::GetItemAdded (Item_ptr srcItem)
	{
		Sync::Payloads_t result;
		
		Sync::Payload item;

		{
			QDataStream ostr (&item.Data_, QIODevice::WriteOnly);
			ostr << static_cast<quint16> (PTItemAdded);
			ostr << srcItem;
		}

		result << item;
		
		return result;
	}
	
	Sync::Payloads_t SyncDeltaGenerator::GetChannelTagsChanged (Channel_ptr channel, const QStringList& tags)
	{
		Sync::Payloads_t result;
		
		Sync::Payload item;
		
		{
		}
	}
}
}
