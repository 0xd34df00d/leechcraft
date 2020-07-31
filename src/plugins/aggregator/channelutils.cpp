/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelutils.h"
#include "storagebackendmanager.h"
#include "storagebackend.h"

namespace LC::Aggregator::ChannelUtils
{
	channels_shorts_t GetAllChannels ()
	{
		channels_shorts_t result;

		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto id : sb->GetFeedsIDs ())
		{
			auto feedChannels = sb->GetChannels (id);
			std::move (feedChannels.begin (), feedChannels.end (), std::back_inserter (result));
		}

		return result;
	}
}
