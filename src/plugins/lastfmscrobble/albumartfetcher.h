/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <QFutureInterface>
#include <util/sll/either.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/media/ialbumartprovider.h>

namespace LC
{
namespace Lastfmscrobble
{
	class AlbumArtFetcher : public QObject
	{
		QFutureInterface<Media::IAlbumArtProvider::Result_t> Promise_;
	public:
		AlbumArtFetcher (const Media::AlbumInfo&, ICoreProxy_ptr, QObject* = nullptr);

		QFuture<Media::IAlbumArtProvider::Result_t> GetFuture ();
	private:
		void HandleReplyFinished (const QByteArray&);
	};
}
}
