/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QtDebug>
#include <numeric>
#include <QVariant>
#include <QStringList>
#include <QPixmap>
#include "channel.h"
#include "item.h"
#include "poolsmanager.h"

namespace LC
{
namespace Aggregator
{
	Channel Channel::CreateForFeed (IDType_t feedId)
	{
		Channel ch;
		ch.ChannelID_ = PoolsManager::Instance ().GetPool (PTChannel).GetID ();
		ch.FeedID_ = feedId;
		return ch;
	}

	int Channel::CountUnreadItems () const
	{
		return std::accumulate (Items_.begin (), Items_.end (),
				0, [] (int cnt, const auto& item) { return cnt + item->Unread_; });
	}

	ChannelShort Channel::ToShort () const
	{
		ChannelShort cs =
		{
			ChannelID_,
			FeedID_,
			Author_,
			Title_,
			DisplayTitle_,
			Link_,
			Tags_,
			LastBuild_,
			Favicon_,
			CountUnreadItems ()
		};
		return cs;
	}

	bool operator< (const ChannelShort& cs1, const ChannelShort& cs2)
	{
		return cs1.ChannelID_ < cs2.ChannelID_;
	}

	bool operator== (const ChannelShort& cs1, const ChannelShort& cs2)
	{
		return cs1.ChannelID_ == cs2.ChannelID_;
	}

	bool operator== (const Channel_ptr& ch, const ChannelShort& cs)
	{
		return ch->ChannelID_ == cs.ChannelID_;
	}

	bool operator== (const ChannelShort& cs, const Channel_ptr& ch)
	{
		return ch == cs;
	}

	bool operator== (const Channel& c1, const Channel& c2)
	{
		return c1.ChannelID_ == c2.ChannelID_;
	}
}
}
