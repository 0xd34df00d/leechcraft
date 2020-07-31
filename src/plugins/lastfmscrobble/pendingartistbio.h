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
#include <interfaces/media/iartistbiofetcher.h>

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class PendingArtistBio : public QObject
	{
		QNetworkAccessManager * const NAM_;
		const bool AddImages_;

		QFutureInterface<Media::IArtistBioFetcher::Result_t> Promise_;
	public:
		PendingArtistBio (QString, QNetworkAccessManager*, bool addImages, QObject* = nullptr);

		QFuture<Media::IArtistBioFetcher::Result_t> GetFuture ();
	private:
		void HandleFinished (const QByteArray&);
	};
}
}
