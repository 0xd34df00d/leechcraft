/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QImage>
#include "item.h"
#include "common.h"

namespace LC
{
namespace Aggregator
{
	struct ChannelShort
	{
		IDType_t ChannelID_ = IDNotFound;
		IDType_t FeedID_ = IDNotFound;
		QString Author_;
		QString Title_;
		QString DisplayTitle_;
		QString Link_;
		QStringList Tags_;
		QDateTime LastBuild_;
		QImage Favicon_;
		int Unread_ = 0;
	};

	struct Channel
	{
		IDType_t ChannelID_ = IDNotFound;
		IDType_t FeedID_ = IDNotFound;
		QString Title_;
		QString DisplayTitle_;
		QString Link_;
		QString Description_;
		QDateTime LastBuild_;
		QStringList Tags_;
		QString Language_;
		QString Author_;
		QString PixmapURL_;
		QImage Pixmap_;
		QImage Favicon_;
		items_container_t Items_;

		static Channel CreateForFeed (IDType_t feedId);

		int CountUnreadItems () const;
		ChannelShort ToShort () const;
	};

	typedef std::shared_ptr<Channel> Channel_ptr;
	typedef std::vector<Channel_ptr> channels_container_t;
	typedef std::vector<ChannelShort> channels_shorts_t;

	bool operator< (const ChannelShort&, const ChannelShort&);
	bool operator== (const ChannelShort&, const ChannelShort&);
	bool operator== (const Channel_ptr&, const ChannelShort&);
	bool operator== (const ChannelShort&, const Channel_ptr&);
	bool operator== (const Channel&, const Channel&);
	QDataStream& operator<< (QDataStream&, const Channel&);
	QDataStream& operator>> (QDataStream&, Channel&);
}
}

Q_DECLARE_METATYPE (LC::Aggregator::ChannelShort)
Q_DECLARE_METATYPE (LC::Aggregator::Channel)
Q_DECLARE_METATYPE (LC::Aggregator::Channel_ptr)
Q_DECLARE_METATYPE (LC::Aggregator::channels_container_t)
