/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include "feed.h"
#include "channel.h"

namespace LC::Aggregator
{
	channels_shorts_t GetAllChannels ();

	class UpdatesManager;

	struct AddFeedParams
	{
		QString URL_;
		QStringList Tags_;
		std::optional<Feed::FeedSettings> FeedSettings_;

		UpdatesManager& UpdatesManager_;
	};
	void AddFeed (const AddFeedParams&);
}
