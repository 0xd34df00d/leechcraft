/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historymanager.h"

namespace LC::HotSensors
{
	const auto PointsCount = 300;

	int HistoryManager::GetMaxHistorySize ()
	{
		return PointsCount;
	}

	void HistoryManager::HandleReadings (const Readings_t& readings)
	{
		for (auto i = History_.begin (); i != History_.end (); )
		{
			const auto pos = std::find_if (readings.begin (), readings.end (),
					[i] (const Reading& r) { return r.Name_ == i.key (); });
			if (pos == readings.end ())
				i = History_.erase (i);
			else
				++i;
		}

		for (const auto& r : readings)
		{
			auto pos = History_.find (r.Name_);
			if (pos == History_.end ())
				pos = History_.insert (r.Name_, Readings_t { PointsCount });
			pos->push_back (r);
		}

		emit historyChanged (History_);
	}
}
