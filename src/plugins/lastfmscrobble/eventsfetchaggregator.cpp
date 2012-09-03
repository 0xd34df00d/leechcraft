/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "eventsfetchaggregator.h"
#include "receventsfetcher.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	EventsFetchAggregator::EventsFetchAggregator (QObject *parent)
	: QObject (parent)
	{
	}

	void EventsFetchAggregator::AddFetcher (RecEventsFetcher *fetcher)
	{
		PendingFetchers_ << fetcher;

		connect (fetcher,
				SIGNAL (gotRecommendedEvents (Media::EventInfos_t)),
				this,
				SLOT (handleGot (Media::EventInfos_t)));
	}

	void EventsFetchAggregator::handleGot (const Media::EventInfos_t& events)
	{
		Aggregated_ << events;
		PendingFetchers_.removeAll (static_cast<RecEventsFetcher*> (sender ()));
		if (!PendingFetchers_.isEmpty ())
			return;

		std::sort (Aggregated_.begin (), Aggregated_.end (),
				[] (decltype (Aggregated_.at (0)) left, decltype ((Aggregated_.at (0))) right)
					{ return left.Date_ < right.Date_; });
		emit gotRecommendedEvents (Aggregated_);
		deleteLater ();
	}
}
}
