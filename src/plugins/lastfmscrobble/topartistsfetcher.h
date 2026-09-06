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
#include <interfaces/media/itopprovider.h>

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class TopArtistsFetcher : public QObject
	{
		QNetworkAccessManager *NAM_;
		QList<Media::TopArtistInfo> Infos_;

		int InfoCount_ = 0;

		QFutureInterface<Media::ITopProvider::TopArtistsResult_t> Promise_;
	public:
		TopArtistsFetcher (QNetworkAccessManager*, QObject* = 0);

		QFuture<Media::ITopProvider::TopArtistsResult_t> GetFuture ();
	private:
		void DecrementWaiting ();
		void HandleFinished (const QByteArray&);
	};
}
}
