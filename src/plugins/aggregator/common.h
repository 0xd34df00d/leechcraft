/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QMetaType>
#include <interfaces/structures.h>

namespace LC::Aggregator
{
	using IDType_t = quint64;

	static constexpr IDType_t IDNotFound = static_cast<IDType_t> (-1);

	using ids_t = QList<IDType_t>;

	enum PoolType
	{
		PTFeed,
		PTChannel,
		PTItem,
		PTEnclosure,
		PTMRSSEntry,
		PTMRSSThumbnail,
		PTMRSSCredit,
		PTMRSSComment,
		PTMRSSPeerLink,
		PTMRSSScene,
		PTMAX
	};

	enum ChannelRoles
	{
		UnreadCount = LC::RoleMAX + 1,
		ErrorCount,
		ChannelID,
		FeedID,
		RawTags,
		HumanReadableTags,
		ChannelTitle,
		ChannelLink,
	};

	extern const QString MessageBoxTitle;

	struct Item;
}

Q_DECLARE_METATYPE (LC::Aggregator::IDType_t)
Q_DECLARE_METATYPE (QList<LC::Aggregator::IDType_t>)
