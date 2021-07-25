/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstdint>

namespace LC::BitTorrent
{
	struct SessionStats
	{
		template<typename T>
		struct Symmetric
		{
			T Down_;
			T Up_;
		};

		Symmetric<int> Rate_;
		Symmetric<int> IPOverheadRate_;
		Symmetric<int> DHTRate_;
		Symmetric<int> TrackerRate_;

		Symmetric<int64_t> Total_;
		Symmetric<int64_t> IPOverheadTotal_;
		Symmetric<int64_t> DHTTotal_;
		Symmetric<int64_t> TrackerTotal_;
		Symmetric<int64_t> PayloadTotal_;

		int NumPeers_;
		int64_t DHTGlobalNodes_;
		int DHTNodes_;
		int DHTTorrents_;

		int64_t TotalFailedBytes_;
		int64_t TotalRedundantBytes_;
	};
}
