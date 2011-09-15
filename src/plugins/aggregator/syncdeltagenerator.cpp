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
#include "core.h"

namespace LeechCraft
{
namespace Aggregator
{
	Sync::Payloads_t SyncDeltaGenerator::GetFeedAdded (Feed_ptr feedPtr)
	{
		Sync::Payloads_t result;
		
		Sync::Payload fp;
		{
			QDataStream ostr (&fp.Data_, QIODevice::WriteOnly);
			ostr << static_cast<quint16> (PTFeedAdded);
			Feed feed (*feedPtr);
			feed.FeedID_ = FixFeedID (feed.FeedID_);
			ostr << feed;
		}
		result << fp;

		return result;
	}

	Sync::Payloads_t SyncDeltaGenerator::GetChanAdded (Channel_ptr channelPtr)
	{
		Sync::Payloads_t result;

		Sync::Payload item;
		
		{
			QDataStream ostr (&item.Data_, QIODevice::WriteOnly);
			ostr << static_cast<quint16> (PTChanAdded);
			Channel channel (*channelPtr);
			channel.ChannelID_ = FixChanID (channel.ChannelID_);
			channel.FeedID_ = FixFeedID (channel.FeedID_);
			ostr << channel;
		}
		
		result << item;

		return result;
	}

	Sync::Payloads_t SyncDeltaGenerator::GetItemAdded (Item_ptr srcItemPtr)
	{
		Sync::Payloads_t result;
		
		Sync::Payload item;

		{
			QDataStream ostr (&item.Data_, QIODevice::WriteOnly);
			ostr << static_cast<quint16> (PTItemAdded);
			Item srcItem (*srcItemPtr);
			srcItem.ItemID_ = FixItemID (srcItem.ItemID_);
			srcItem.ChannelID_ = FixChanID (srcItem.ChannelID_);
			ostr << srcItem;
		}

		result << item;
		
		return result;
	}
	
	Sync::Payloads_t SyncDeltaGenerator::GetChannelTagsChanged (Channel_ptr channel, const QStringList& tags)
	{
		Sync::Payloads_t result;
		
		Sync::Payload item;
		
		
		return result;
	}
	
	void SyncDeltaGenerator::ParseDelta (const Sync::Payload& delta)
	{
		QDataStream istr (delta.Data_);
		quint16 action = 0;
		istr >> action;
		
		switch (action)
		{
		case PTFeedAdded:
		{
			Feed_ptr feed (new Feed);
			istr >> *feed;
			if (!Remote2LocalFeeds_.contains (feed->FeedID_))
			{
				IDType_t newID = Core::Instance ().GetStorageBackend ()->GetHighestID (PTFeed);
				Remote2LocalFeeds_ [feed->FeedID_] = newID;
			}
			feed->FeedID_ = Remote2LocalFeeds_ [feed->FeedID_];

			feeds_container_t feeds;
			feeds.push_back (feed);
			Core::Instance ().AddFeeds (feeds, QString ());
			break;
		}
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown action"
					<< action;
			return;
		}
	}
	
	IDType_t SyncDeltaGenerator::FixFeedID (IDType_t id)
	{
		return Remote2LocalFeeds_.key (id, id);
	}
	
	IDType_t SyncDeltaGenerator::FixChanID (IDType_t id)
	{
		return Remote2LocalChannels_.key (id, id);
	}
	
	IDType_t SyncDeltaGenerator::FixItemID (IDType_t id)
	{
		return Remote2LocalItems_.key (id, id);
	}
}
}
