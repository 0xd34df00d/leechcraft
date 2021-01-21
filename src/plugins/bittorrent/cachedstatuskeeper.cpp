/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cachedstatuskeeper.h"

namespace LC::BitTorrent
{
	libtorrent::torrent_status CachedStatusKeeper::GetStatus (const libtorrent::torrent_handle& handle, FlagsType_t flags)
	{
		if (Handle2Status_.contains (handle))
		{
			const auto& item = Handle2Status_ [handle];
			if ((item.ReqFlags_ & flags) == flags)
				return item.Status_;

			flags |= item.ReqFlags_;
		}

		const auto& status = handle.status (flags);
		Handle2Status_ [handle] = { status, flags };
		return status;
	}

	void CachedStatusKeeper::HandleStatusUpdatePosted (const libtorrent::torrent_status& status)
	{
		Handle2Status_ [status.handle] = { status, AllFlags };
	}
}
