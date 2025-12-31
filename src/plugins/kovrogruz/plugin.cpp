/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "plugin.h"
#include <QIcon>
#include <QUrlQuery>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/channelutils.h>
#include <util/sll/json.h>

namespace LC::Kovrogruz
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Kovrogruz"_qba;
	}

	QString Plugin::GetName () const
	{
		return "Kovrogruz"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Fetches album art from services like iTunes Music.");
	}

	void Plugin::Release ()
	{
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	namespace
	{
		using Channel_t = Media::IAlbumArtProvider::Channel_t;

		QUrl MakeUrl (QString artwork100str)
		{
			artwork100str.replace ("100x100bb"_ql, "2000x2000bb"_ql);
			return QUrl { artwork100str };
		}

		Util::Task<Util::Either<QString, QList<QUrl>>> QueryITunesReturning (Media::AlbumInfo album)
		{
			QUrl url { "https://itunes.apple.com/search"_qs };
			url.setQuery ({
					{ "entity"_qs, "album"_qs },
					{ "limit"_qs, "5"_qs },
					{ "term"_qs, album.Artist_ + ' ' + album.Album_ },
				});

			const auto reply = co_await *GetProxyHolder ()->GetNetworkAccessManager ()->get (QNetworkRequest { url });
			const auto& data = co_await reply.ToEither ();
			const auto& json = co_await Util::ToJson (data);

			try
			{
				using enum QJsonValue::Type;
				using Util::As;

				QList<QUrl> urls;

				const auto& root = As<Object> (json);
				for (const auto& resultVal : As<Array> (root ["results"_ql]))
				{
					const auto& result = As<Object> (resultVal);
					urls << MakeUrl (As<String> (result ["artworkUrl100"_ql]));
				}

				co_return urls;
			}
			catch (const Util::UnexpectedJson& error)
			{
				qWarning () << "cannot get release IDs:" << error.what ();
				co_return { Util::AsLeft, QObject::tr ("Unexpected JSON contents.") };
			}
		}

		Util::Task<void> QueryITunes(Channel_t chan, Media::AlbumInfo album)
		{
			const auto service = "iTunes"_qs;

			const auto result = co_await QueryITunesReturning (album);
			Util::Visit (result,
					[&] (const QString& error) { chan->Send ({ service, { Util::AsLeft, error } }); },
					[&] (const QList<QUrl>& urls) { chan->Send ({ service, urls }); });
		}
	}

	Media::IAlbumArtProvider::Channel_t Plugin::RequestAlbumArt (const Media::AlbumInfo& album) const
	{
		return Util::WithChannel ([] (Channel_t channel, const Media::AlbumInfo& album)
				{
					return Util::InParallel ({ QueryITunes (channel, album) });
				}, album);
	}
}

LC_EXPORT_PLUGIN (leechcraft_kovrogruz, LC::Kovrogruz::Plugin);
