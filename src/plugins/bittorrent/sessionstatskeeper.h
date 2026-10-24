/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <libtorrent/fwd.hpp>
#include <libtorrent/time.hpp>
#include "sessionstats.h"

namespace LC::BitTorrent
{
	class AlertDispatcher;

	// TODO make updates event-driven and emit a signal on updates from here instead of timers in consumers
	class SessionStatsKeeper
	{
		struct Sample
		{
			libtorrent::time_point Time_;
			SessionStats::Symmetric<int64_t> Total_;
			SessionStats::Symmetric<int64_t> IPOverheadTotal_;
			SessionStats::Symmetric<int64_t> DHTTotal_;
			SessionStats::Symmetric<int64_t> TrackerTotal_;
		};
		std::optional<Sample> Prev_;

		SessionStats Stats_ {};
	public:
		explicit SessionStatsKeeper (AlertDispatcher&);

		const SessionStats& GetStats () const;
	private:
		void HandleStats (const libtorrent::session_stats_alert&);
	};
}
