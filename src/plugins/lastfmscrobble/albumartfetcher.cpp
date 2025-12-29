/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumartfetcher.h"
#include <QStringList>
#include <interfaces/core/icoreproxy.h>
#include <util/threads/coro.h>
#include <util/threads/coro/asdomdocument.h>
#include <util/threads/coro/channelutils.h>
#include <util/sll/qtutil.h>
#include "util.h"

namespace LC::Lastfmscrobble
{
	namespace
	{
		Util::Task<Util::Either<QString, std::optional<QUrl>>> RunFetch (Media::AlbumInfo albumInfo)
		{
			const QMap<QString, QString> params
			{
				{ "artist"_qs, albumInfo.Artist_ },
				{ "album"_qs, albumInfo.Album_ },
				{ "autocorrect"_qs, "1"_qs }
			};

			const auto reply = co_await *Request ("album.getInfo"_qs, GetProxyHolder ()->GetNetworkAccessManager (), params);
			const auto replyData = co_await reply.ToEither ();
			const auto doc = co_await Util::AsDomDocument { replyData, QObject::tr ("Unable to parse reply.") };

			const auto& elems = doc.elementsByTagName ("image"_qs);

			static const QStringList sizes
			{
				"mega"_qs,
				"extralarge"_qs,
				"large"_qs,
				"medium"_qs,
				"small"_qs,
				{}
			};

			for (const auto& size : sizes)
				for (const auto & node : elems)
				{
					const auto& elem = node.toElement ();
					if (const auto& text = elem.text ();
						elem.attribute ("size"_qs) == size && !text.isEmpty ())
						co_return std::optional { QUrl { text } };
				}

			co_return std::optional<QUrl> {};
		}

		using Result_t = Media::IAlbumArtProvider::AlbumArtResponse;

		Result_t Convert (const Util::Either<QString, std::optional<QUrl>>& result)
		{
			static const auto service = "Last.FM"_qs;
			return Util::Visit (result,
					[] (const QString& error) { return Result_t { service, { Util::AsLeft, error } }; },
					[] (const std::optional<QUrl>& url) -> Result_t
					{
						QList<QUrl> urls;
						if (url)
							urls << *url;
						return Result_t { service, urls };
					});
		}
	}

	Util::Channel_ptr<Result_t> FetchAlbumArt (const Media::AlbumInfo& albumInfo)
	{
		return Util::ChannelFromSingleResult (&RunFetch, &Convert, albumInfo);
	}
}
