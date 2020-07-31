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
#include <interfaces/media/ihypesprovider.h>

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class HypedTracksFetcher : public QObject
	{
		QFutureInterface<Media::IHypesProvider::HypeQueryResult_t> Promise_;
	public:
		HypedTracksFetcher (QNetworkAccessManager*, Media::IHypesProvider::HypeType, QObject* = 0);

		QFuture<Media::IHypesProvider::HypeQueryResult_t> GetFuture ();
	private:
		void HandleFinished (const QByteArray&);
	};
}
}
