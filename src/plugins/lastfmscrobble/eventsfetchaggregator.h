/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFutureInterface>
#include <interfaces/media/ieventsprovider.h>
#include <util/sll/either.h>

namespace LC
{
namespace Lastfmscrobble
{
	class RecEventsFetcher;

	class EventsFetchAggregator : public QObject
	{
		QList<RecEventsFetcher*> PendingFetchers_;
		Media::EventInfos_t Aggregated_;

		QFutureInterface<Media::IEventsProvider::EventsQueryResult_t> Promise_;
	public:
		EventsFetchAggregator (QObject* = nullptr);

		QFuture<Media::IEventsProvider::EventsQueryResult_t> GetFuture ();

		void AddFetcher (RecEventsFetcher*);
	};
}
}
