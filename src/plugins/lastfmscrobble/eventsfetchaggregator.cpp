/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eventsfetchaggregator.h"
#include <algorithm>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include "receventsfetcher.h"

namespace LC
{
namespace Lastfmscrobble
{
	EventsFetchAggregator::EventsFetchAggregator (QObject *parent)
	: QObject { parent }
	{
		Promise_.reportStarted ();
	}

	QFuture<Media::IEventsProvider::EventsQueryResult_t> EventsFetchAggregator::GetFuture ()
	{
		return Promise_.future ();
	}

	void EventsFetchAggregator::AddFetcher (RecEventsFetcher *fetcher)
	{
		PendingFetchers_ << fetcher;

		connect (fetcher,
				&RecEventsFetcher::gotRecommendedEvents,
				this,
				[this, fetcher] (const Media::EventInfos_t& events)
				{
					Aggregated_ << events;
					PendingFetchers_.removeOne (fetcher);
					if (!PendingFetchers_.isEmpty ())
						return;

					std::sort (Aggregated_.begin (), Aggregated_.end (), Util::ComparingBy (&Media::EventInfo::Date_));
					Util::ReportFutureResult (Promise_, Aggregated_);
					deleteLater ();
				});
	}
}
}
