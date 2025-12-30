/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumartfetcher.h"
#include <QJsonDocument>
#include <QUrlQuery>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/either.h>
#include <util/sll/json.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/channelutils.h>
#include <util/threads/coro/throttle.h>
#include "util.h"

namespace LC::MusicZombie
{
	namespace
	{
		QNetworkRequest MakeReleaseQueryRequest (Media::AlbumInfo info)
		{
			info.Artist_.remove ('"');
			info.Album_.remove ('"');

			QUrl url { "http://musicbrainz.org/ws/2/release"_qs };
			url.setQuery ({
					{ "query"_qs, R"(artist:"%1" AND release:"%2")"_qs.arg (info.Artist_, info.Album_) },
					{ "fmt"_qs, "json"_qs },
				});
			return SetupRequest (QNetworkRequest { url });
		}

		QNetworkRequest MakeAlbumArtQueryRequest (const QString& releaseId)
		{
			QNetworkRequest req { QUrl { "http://coverartarchive.org/release/%1/front"_qs.arg (releaseId) } };
			req.setAttribute (QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
			return SetupRequest (req);
		}

		Util::Either<QString, QStringList> GetReleaseIds (const QJsonDocument& doc)
		{
			const auto& errorMessage = QObject::tr ("Unexpected JSON contents.");

			QStringList ids;
			try
			{
				using enum QJsonValue::Type;
				using Util::As;

				const auto& root = As<Object> (doc);
				for (const auto& releaseVal : As<Array> (root ["releases"_ql]))
				{
					const auto& release = As<Object> (releaseVal);
					const auto& score = As<Double> (release ["score"_ql]);
					constexpr auto scoreThreshold = 90;
					if (score >= scoreThreshold)
						ids << As<String> (release ["id"_ql]);
				}
			}
			catch (const Util::UnexpectedJson& error)
			{
				qWarning () << "cannot get release IDs:" << error.what ();
				return { Util::AsLeft, errorMessage };
			}

			return ids;
		}

		Util::Task<Util::NetworkResult> BackingOff (std::function<QNetworkReply* ()> doRequest,
				std::shared_ptr<Util::Throttle> throttle,
				int attempts = 3)
		{
			Util::NetworkResult reply;
			for (int i = 0; i < attempts; ++i)
			{
				co_await *throttle;

				reply = co_await *doRequest ();
				if (const auto error = reply.IsError ();
					error && error->Error_ == QNetworkReply::ServiceUnavailableError)
				{
					qDebug () << "backing off after" << reply;
					throttle->Backoff ();
					continue;
				}

				co_return reply;
			}

			co_return reply;
		}

		Util::Task<void> RunFetch (Media::IAlbumArtProvider::Channel_t chan,
				Media::AlbumInfo info, std::shared_ptr<Util::Throttle> throttle)
		{
			auto& nam = *GetProxyHolder ()->GetNetworkAccessManager ();
			const auto& releaseQueryReply = co_await BackingOff ([&] { return nam.get (MakeReleaseQueryRequest (info)); }, throttle);
			const auto& data = co_await releaseQueryReply.ToEither (QObject::tr ("Unable to query releases."));
			const auto& json = co_await Util::ToJson (data);
			const auto& ids = co_await GetReleaseIds (json);
			for (const auto& id : ids)
			{
				co_await *throttle;
				const auto reply = nam.head (MakeAlbumArtQueryRequest (id));
				if (const auto error = (co_await *reply).IsError ())
				{
					if (error->Error_ == QNetworkReply::ContentNotFoundError)
						qDebug () << "no album art for release" << id;
					else
						qWarning () << *error;
					continue;
				}
				chan->Send ({ "MusicBrainz"_qs, { { reply->url () } }});
			}
		}
	}

	Media::IAlbumArtProvider::Channel_t FetchAlbumArt (const Media::AlbumInfo& info,
			const std::shared_ptr<Util::Throttle>& throttle)
	{
		return Util::WithChannel (&RunFetch, info, throttle);
	}
}
