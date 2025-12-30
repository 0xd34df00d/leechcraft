/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <util/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/either.h>
#include <util/threads/coro.h>
#include <util/threads/coro/channelutils.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/ialbumartprovider.h>
#include <util/lmp/mediainfo.h>

namespace LC
{
namespace LMP
{
	QMap<QString, std::function<QString (MediaInfo)>> GetSubstGetters ()
	{
		static const QMap<QString, std::function<QString (MediaInfo)>> map
		{
			{ "$artist", [] (const MediaInfo& info) { return info.Artist_; } },
			{ "$album", [] (const MediaInfo& info) { return info.Album_; } },
			{ "$title", [] (const MediaInfo& info) { return info.Title_; } },
			{ "$year", [] (const MediaInfo& info) { return QString::number (info.Year_); } },
			{ "$trackNumber", [] (const MediaInfo& info) -> QString
				{
					auto trackNumStr = QString::number (info.TrackNumber_);
					if (info.TrackNumber_ < 10)
						trackNumStr.prepend ('0');
					return trackNumStr;
				} }
		};
		return map;
	}

	QMap<QString, std::function<void (MediaInfo&, QString)>> GetSubstSetters ()
	{
		static const QMap<QString, std::function<void (MediaInfo&, QString)>> map
		{
			{ "$artist", [] (MediaInfo& info, const QString& val) { info.Artist_ = val; } },
			{ "$album", [] (MediaInfo& info, const QString& val) { info.Album_= val; } },
			{ "$title", [] (MediaInfo& info, const QString& val) { info.Title_ = val; } },
			{ "$year", [] (MediaInfo& info, const QString& val) { info.Year_ = val.toInt (); } },
			{ "$trackNumber", [] (MediaInfo& info, QString val)
				{
					if (val.size () == 2 && val.at (0) == '0')
						val = val.mid (1);
					info.TrackNumber_ = val.toInt ();
				} }
		};
		return map;
	}

	QStringList GetSubstGettersKeys ()
	{
		static const QStringList keys = GetSubstGetters ().keys ();
		return keys;
	}

	QString PerformSubstitutions (QString mask, const MediaInfo& info, SubstitutionFlags flags)
	{
		for (const auto& pair : Util::Stlize (GetSubstGetters ()))
		{
			auto value = pair.second (info);
			if (flags & SubstitutionFlag::SFSafeFilesystem)
				value.replace ('/', '_');
			mask.replace (pair.first, value);
		}

		if (flags & SubstitutionFlag::SFSafeFilesystem)
		{
			mask.replace ('?', '_');
			mask.replace ('*', '_');
		}

		return mask;
	}

	QStringList PerformSubstitutions (const QString& pattern,
			const QList<MediaInfo>& infos,
			const std::function<void (int, QString)>& setter,
			SubstitutionFlags flags)
	{
		QStringList names;

		const bool hasExtension = pattern.contains ('.');

		int row = 0;
		for (const auto& info : infos)
		{
			auto name = PerformSubstitutions (pattern, info, flags);
			if (!hasExtension)
				name += '.' + info.LocalPath_.section ('.', -1);

			names << name;

			setter (row++, name);
		}

		return names;
	}

	namespace
	{
		Util::Task<void> FetchImage (Util::Channel_ptr<AlbumArtInfo<QImage>> channel, ICoreProxy_ptr proxy, AlbumArtInfo<QUrl> info)
		{
			const auto nam = proxy->GetNetworkAccessManager ();
			const auto reply = co_await *nam->get (QNetworkRequest { info.AlbumArt_ });
			const auto data = co_await reply.ToEither ();

			QImage image;
			if (!image.loadFromData (data))
			{
				qWarning () << "unable to decode image from" << info.AlbumArt_;
				co_return;
			}

			channel->Send ({ info.Service_, std::move (image) });
		}

		Util::Task<void> RunGetAlbumArtUrls (Util::Channel_ptr<AlbumArtInfo<QUrl>> outChan,
				ICoreProxy_ptr proxy, QString artist, QString album)
		{
			QVector<Media::IAlbumArtProvider::Channel_t> channels;
			for (const auto prov : proxy->GetPluginsManager ()->GetAllCastableTo<Media::IAlbumArtProvider*> ())
				channels << prov->RequestAlbumArt ({ artist, album });

			const auto& merged = Util::MergeChannels (channels);

			while (const auto& response = co_await *merged)
			{
				const auto urls = co_await Util::WithHandler (response->Result_, Util::IgnoreLeft {});
				for (const auto& url : urls)
					outChan->Send ({ response->ServiceName_, url });
			}
		}

		Util::Task<void> RunGetAlbumArtImages (Util::Channel_ptr<AlbumArtInfo<QImage>> outChan,
				ICoreProxy_ptr proxy, Util::Channel_ptr<AlbumArtInfo<QUrl>> urlsChan)
		{
			while (const auto& info = co_await *urlsChan)
				co_await FetchImage (outChan, proxy, *info);
		}
	}

	Util::Channel_ptr<AlbumArtInfo<QUrl>> GetAlbumArtUrls (const ICoreProxy_ptr& proxy, const QString& artist, const QString& album)
	{
		return Util::WithChannel (&RunGetAlbumArtUrls, proxy, artist, album);
	}

	Util::Channel_ptr<AlbumArtInfo<QImage>> GetAlbumArtImages (const ICoreProxy_ptr& proxy, const QString& artist, const QString& album)
	{
		return Util::WithChannel (&RunGetAlbumArtImages, proxy, GetAlbumArtUrls (proxy, artist, album));
	}
}
}
