/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "resourcesfetcher.h"
#include <interfaces/core/ientitymanager.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include "storagebackendmanager.h"

namespace LC::Aggregator
{
	namespace
	{
		Util::Task<Util::Either<QString, QImage>> FetchImage (const QUrl& url)
		{
			const auto result = co_await *GetProxyHolder ()->GetNetworkAccessManager ()->get (QNetworkRequest { url });
			const auto imageData = co_await result.ToEither ();

			const auto image = QImage::fromData (imageData);
			if (image.isNull ())
				co_return Util::Left { QObject::tr ("Empty image data") };

			co_return image;
		}
	}

	Util::Task<Util::Either<QString, Util::Void>> UpdateFavicon (IDType_t channelId, const QString& channelLink)
	{
		QUrl url { channelLink };
		url.setPath ("/favicon.ico"_qs);
		const auto image = co_await co_await FetchImage (url);

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->SetChannelFavicon (channelId, image);

		co_return Util::Void {};
	}

	Util::Task<Util::Either<QString, Util::Void>> UpdatePixmap (IDType_t channelId, const QString& pixmapLink)
	{
		const QUrl url { pixmapLink };
		if (!url.isValid ())
			co_return Util::Left { QObject::tr ("Empty pixmap link") };

		const auto image = co_await co_await FetchImage (QUrl { pixmapLink });

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->SetChannelPixmap (channelId, image);

		co_return Util::Void {};
	}

	void UpdateChannelResources (const Channel& channel)
	{
		UpdateFavicon (channel.ChannelID_, channel.Link_);
		UpdatePixmap (channel.ChannelID_, channel.PixmapURL_);
	}
}
