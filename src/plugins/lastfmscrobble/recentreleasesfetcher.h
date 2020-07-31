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
#include <util/sll/either.h>
#include <interfaces/media/irecentreleases.h>

namespace Media
{
	struct AlbumRelease;
}

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class RecentReleasesFetcher : public QObject
	{
		QFutureInterface<Media::IRecentReleases::Result_t> Promise_;
	public:
		RecentReleasesFetcher (bool, QNetworkAccessManager*, QObject* = nullptr);

		QFuture<Media::IRecentReleases::Result_t> GetFuture ();
	private:
		void HandleData (const QByteArray&);
	};
}
}
