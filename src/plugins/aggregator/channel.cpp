/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QtDebug>
#include <numeric>
#include <QDataStream>
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

	QDataStream& operator<< (QDataStream& out, const Channel& chan)
	{
		int version = 4;
		out << version
			<< chan.Title_
			<< chan.DisplayTitle_
			<< chan.Link_
			<< chan.Description_
			<< chan.LastBuild_
			<< chan.Tags_
			<< chan.Language_
			<< chan.Author_
			<< chan.PixmapURL_
			<< chan.Pixmap_
			<< chan.Favicon_
			<< static_cast<quint32> (chan.Items_.size ());
		for (const auto& item : chan.Items_)
			out << *item;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Channel& chan)
	{
		int version = 0;
		in >> version;
		if (version < 1 || version > 4)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		in >> chan.Title_;

		if (version == 4)
			in >> chan.DisplayTitle_;

		in >> chan.Link_
			>> chan.Description_
			>> chan.LastBuild_
			>> chan.Tags_
			>> chan.Language_
			>> chan.Author_
			>> chan.PixmapURL_;

		if (version == 1)
		{
			quint32 size;
			in >> chan.Pixmap_
				>> chan.Favicon_;
			in >> size;
			for (size_t i = 0; i < size; ++i)
			{
				auto it = std::make_shared<Item> ();
				in >> *it;
				chan.Items_.push_back (it);
			}
		}
		else
		{
			quint32 size;
			if (version == 3)
				in >> chan.Pixmap_
					>> chan.Favicon_;
			else
			{
				QPixmap px, favicon;
				in >> px
					>> favicon;
				chan.Pixmap_ = px.toImage ();
				chan.Favicon_ = favicon.toImage ();
			}

			in >> size;
			for (size_t i = 0; i < size; ++i)
			{
				auto it = std::make_shared<Item> ();
				in >> *it;
				chan.Items_.push_back (it);
			}
		}

		return in;
	}
}
}
