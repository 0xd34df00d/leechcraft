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

#ifndef PLUGINS_AGGREGATOR_CHANNEL_H
#define PLUGINS_AGGREGATOR_CHANNEL_H
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QPixmap>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "item.h"
#include "common.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct ChannelShort
			{
				IDType_t ChannelID_;
				IDType_t FeedID_;
				QString Author_;
				QString Title_;
				QString Link_;
				QStringList Tags_;
				QDateTime LastBuild_;
				QPixmap Favicon_;
				int Unread_;
			};

			struct Channel
			{
				IDType_t ChannelID_;
				IDType_t FeedID_;
				QString Title_;
				QString Link_;
				QString Description_;
				QDateTime LastBuild_;
				QStringList Tags_;
				QString Language_;
				QString Author_;
				QString PixmapURL_;
				QPixmap Pixmap_;
				QPixmap Favicon_;
				items_container_t Items_;

				Channel (const IDType_t& feedId);
				Channel (const IDType_t& feedId, const IDType_t& channelId);
				Channel (const Channel&);
				Channel& operator= (const Channel&);

				int CountUnreadItems () const;
				void Equalify (const Channel&);
				ChannelShort ToShort () const;
			};

			typedef boost::shared_ptr<Channel> Channel_ptr;
			typedef std::vector<Channel_ptr> channels_container_t;
			typedef std::vector<ChannelShort> channels_shorts_t;

			bool operator< (const ChannelShort&, const ChannelShort&);
			bool operator== (const ChannelShort&, const ChannelShort&);
			bool operator== (const Channel_ptr&, const ChannelShort&);
			bool operator== (const ChannelShort&, const Channel_ptr&);
			bool operator== (const Channel&, const Channel&);
			QDataStream& operator<< (QDataStream&, const Channel&);
			QDataStream& operator>> (QDataStream&, Channel&);
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Aggregator::Channel_ptr);

#endif

