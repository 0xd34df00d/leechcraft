/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumartmanager.h"
#include <functional>
#include <QUrl>
#include <QtConcurrentRun>
#include <QFuture>
#include <util/sll/either.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/media/ialbumartprovider.h>
#include "literals.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"

namespace LC::LMP
{
	AlbumArtManager::AlbumArtManager (LocalCollection& coll, QObject *parent)
	: QObject { parent }
	, Collection_ { coll }
	{
		XmlSettingsManager::Instance ().RegisterObject ("CoversStoragePath", this,
				[this] (const QVariant& var) { HandleCoversPath (var.toString ()); });

		connect (&coll,
				&LocalCollection::gotNewArtists,
				this,
				&AlbumArtManager::CheckNewArtists);
	}

	namespace
	{
		Util::Task<void> FetchImage (Util::Channel_ptr<QImage> channel, const QUrl& url)
		{
			const auto nam = GetProxyHolder ()->GetNetworkAccessManager ();
			const auto reply = co_await *nam->get (QNetworkRequest { url });
			const auto data = co_await reply.ToEither ();

			QImage image;
			if (!image.loadFromData (data))
			{
				qWarning () << "unable to decode image from" << url;
				co_return;
			}

			channel->Send (std::move (image));
		}

		Util::Task<void> RunCheckAlbumArt (Util::Channel_ptr<QImage> channel, QString artist, QString album)
		{
			for (const auto prov : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::IAlbumArtProvider*> ())
			{
				const auto eitherUrls = co_await prov->RequestAlbumArt ({ artist, album });
				const auto urls = co_await eitherUrls;
				for (const auto& url : urls)
					co_await FetchImage (channel, url);
			}

			channel->Close ();
		}
	}

	Util::Channel_ptr<QImage> AlbumArtManager::CheckAlbumArt (const QString& artist, const QString& album)
	{
		auto channel = std::make_shared<Util::Channel<QImage>> ();
		RunCheckAlbumArt (channel, artist, album);
		return channel;
	}

	void AlbumArtManager::SetAlbumArt (int id, const QString& artist, const QString& album, const QImage& image)
	{
		auto joined = artist + "_-_" + album;
		joined.replace (' ', '_');
		const auto& filename = QUrl::toPercentEncoding (joined, QByteArray (), "~") + ".png";
		const auto& fullPath = AADir_.absoluteFilePath (filename);

		constexpr auto compressionRatio = 100;
		QtConcurrent::run ([image, fullPath] { return image.save (fullPath, "PNG", compressionRatio); })
			.then (this, [=, this] (bool saved)
				{
					if (saved)
						Collection_.SetAlbumArt (id, fullPath);
					else
						qWarning () << "unable to save album art to" << fullPath;
				});
	}

	namespace
	{
		const auto NotFoundMarker = "NOTFOUND"_qs;
	}

	Util::ContextTask<void> AlbumArtManager::CheckNewArtists (Collection::Artists_t artists)
	{
		if (!XmlSettingsManager::Instance ().property ("AutoFetchAlbumArt").toBool ())
			co_return;

		co_await Util::AddContextObject { *this };

		for (const auto& artist : artists)
			for (const auto& album : artist.Albums_)
				if (album->CoverPath_.isEmpty () || album->CoverPath_ == NotFoundMarker)
				{
					const auto channel = CheckAlbumArt (artist.Name_, album->Name_);
					QList<QImage> images;
					while (const auto image = co_await *channel)
						images << *image;

					if (images.isEmpty ())
						Collection_.SetAlbumArt (album->ID_, NotFoundMarker);
					else
						SetAlbumArt (album->ID_, artist.Name_, album->Name_,
								*std::ranges::max_element (images, Util::ComparingBy (&QImage::width)));
				}
	}

	void AlbumArtManager::HandleCoversPath (const QString& path)
	{
		bool failed = false;
		if (!QFile::exists (path) && !QDir::root ().mkpath (path))
			failed = true;

		const QFileInfo fi (path);
		if (!failed && !(fi.isDir () && fi.isReadable ()))
			failed = true;

		if (failed)
		{
			qWarning () << "unable to create" << path;
			const auto& e = Util::MakeNotification (Lits::LMP,
					tr ("Path %1 cannot be used as album art storage, default path will be used instead."),
					Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
			AADir_ = Util::GetUserDir (Util::UserDir::Cache, "lmp/covers"_qs);
		}
		else
			AADir_ = QDir (path);
	}
}
