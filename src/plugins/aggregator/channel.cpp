/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include <QtDebug>
#include <QDataStream>
#include <QVariant>
#include <QStringList>
#include "channel.h"
#include "item.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Channel::Channel (const IDType_t& id)
			: ChannelID_ (Core::Instance ().GetPool (Core::PTChannel).GetID ())
			, FeedID_ (id)
			{
			}

			Channel::Channel (const IDType_t& id, const IDType_t& chId)
			: ChannelID_ (chId)
			, FeedID_ (id)
			{
			}

			Channel::Channel (const Channel& channel)
			: Items_ (channel.Items_)
			{
				Equalify (channel);
			}
			
			Channel& Channel::operator= (const Channel& channel)
			{
				Equalify (channel);
				Items_ = channel.Items_;
				return *this;
			}
			
			int Channel::CountUnreadItems () const
			{
				int result = 0;
				for (size_t i = 0; i < Items_.size (); ++i)
					result += (Items_ [i]->Unread_);
				return result;
			}
			
			void Channel::Equalify (const Channel& channel)
			{
				ChannelID_ = channel.ChannelID_;
				FeedID_ = channel.FeedID_;
				Title_ = channel.Title_;
				Link_ = channel.Link_;
				Description_ = channel.Description_;
				LastBuild_ = channel.LastBuild_;
				Tags_ = channel.Tags_;
				Language_ = channel.Language_;
				Author_ = channel.Author_;
				PixmapURL_ = channel.PixmapURL_;
				Pixmap_ = channel.Pixmap_;
				Favicon_ = channel.Favicon_;
			}
			
			ChannelShort Channel::ToShort () const
			{
				ChannelShort cs =
				{
					ChannelID_,
					FeedID_,
					Title_,
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
				int version = 2;
				out << version
					<< chan.ChannelID_
					<< chan.FeedID_
					<< chan.Title_
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
				for (size_t i = 0; i < chan.Items_.size (); ++i)
					out << *chan.Items_ [i];
				return out;
			}

			QDataStream& operator>> (QDataStream& in, Channel& chan)
			{
				int version = 0;
				in >> version;
				if (version == 1)
				{
					quint32 size;
					in >> chan.Title_
						>> chan.Link_
						>> chan.Description_
						>> chan.LastBuild_
						>> chan.Tags_
						>> chan.Language_
						>> chan.Author_
						>> chan.PixmapURL_
						>> chan.Pixmap_
						>> chan.Favicon_;
					in >> size;
					for (size_t i = 0; i < size; ++i)
					{
						Item_ptr it (new Item (chan.ChannelID_));
						in >> *it;
						chan.Items_.push_back (it);
					}
				}
				else if (version == 2)
				{
					quint32 size;
					in >> chan.ChannelID_
						>> chan.FeedID_
						>> chan.Title_
						>> chan.Link_
						>> chan.Description_
						>> chan.LastBuild_
						>> chan.Tags_
						>> chan.Language_
						>> chan.Author_
						>> chan.PixmapURL_
						>> chan.Pixmap_
						>> chan.Favicon_;
					in >> size;
					for (size_t i = 0; i < size; ++i)
					{
						Item_ptr it (new Item (chan.ChannelID_));
						in >> *it;
						chan.Items_.push_back (it);
					}
				}
				return in;
			}
		};
	};
};

