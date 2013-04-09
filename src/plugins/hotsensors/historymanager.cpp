/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "historymanager.h"

namespace LeechCraft
{
namespace HotSensors
{
	HistoryManager::HistoryManager (QObject *parent)
	: QObject (parent)
	{
	}

	void HistoryManager::handleReadings (const Readings_t& readings)
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
			auto& vec = History_ [r.Name_];
			vec << r.Value_;
			if (vec.size () >= 100)
				vec.pop_front ();
		}
	}
}
}
