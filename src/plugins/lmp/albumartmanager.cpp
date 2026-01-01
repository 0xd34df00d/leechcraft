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
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include <util/lmp/util.h>
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
		const auto NotFoundMarker = "NOTFOUND"_qs;

		QString MakeCoverFileName (const QString& artist, const QString& album)
		{
			auto joined = artist + "_-_"_qs + album;
			joined.replace (' ', '_');
			return QUrl::toPercentEncoding (joined, QByteArray (), "~") + ".png";
		}

		QString GetCoverPath (const QString& artist, const Collection::Album& album, const QDir& aaDir)
		{
			const auto& filename = MakeCoverFileName (artist, album.Name_);
			const auto& fullPath = aaDir.absoluteFilePath (filename);
			return fullPath;
		}
	}

	void AlbumArtManager::SetAlbumArt (const QString& artist, const Collection::Album& album, const QImage& image)
	{
		const auto& fullPath = GetCoverPath (artist, album, AADir_);
		qDebug () << "saving AA for" << album.Name_ << "to" << fullPath;

		constexpr auto compressionRatio = 100;
		QtConcurrent::run ([image, fullPath] { return image.save (fullPath, "PNG", compressionRatio); })
			.then (this, [fullPath, id = album.ID_, this] (bool saved)
				{
					if (saved)
						Collection_.SetAlbumArt (id, fullPath);
					else
						qWarning () << "unable to save album art to" << fullPath;
				});
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
					const auto channel = GetAlbumArtImages (GetProxyHolder (), artist.Name_, album->Name_);
					QList<QImage> images;
					while (const auto result = co_await *channel)
						images << result->AlbumArt_;

					if (images.isEmpty ())
						Collection_.SetAlbumArt (album->ID_, NotFoundMarker);
					else
						SetAlbumArt (artist.Name_, *album,
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
