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

		std::optional<QString> GetCurrentCoverPath (const Collection::Album& album)
		{
			const auto& curPath = album.CoverPath_;
			if (curPath.isEmpty () || curPath == NotFoundMarker)
				return {};

			if (const QFileInfo fi { curPath };
				!fi.exists () || !fi.isWritable ())
				return {};

			return curPath;
		}

		QString MakeCoverFileName (const QString& artist, const QString& album)
		{
			auto joined = artist + "_-_"_qs + album;
			joined.replace (' ', '_');
			return QUrl::toPercentEncoding (joined, QByteArray (), "~") + ".png";
		}

		std::optional<QString> GetAlbumDirCoverPath (const Collection::Album& album, const QString& artist)
		{
			const auto& tracks = album.Tracks_;
			if (tracks.isEmpty ())
				return {};

			const auto trackDirPath = [] (const Collection::Track& track)
			{
				return QFileInfo { track.FilePath_ }.dir ().canonicalPath ();
			};
			const auto& albumDir = trackDirPath (tracks [0]);

			if (!QFileInfo { albumDir }.isWritable ())
				return {};

			if (!std::ranges::all_of (tracks, [&] (const auto& track) { return trackDirPath (track) == albumDir; }))
				return {};

			const QDir dir { albumDir };
			if (dir.entryList ().size () > tracks.size () * 2 + 10)
			{
				qWarning () << "target dir" << albumDir << "is a mess, not saving AA there";
				return {};
			}

			if (const auto defaultName = "cover.png"_qs;
				!dir.exists (defaultName))
				return dir.filePath (defaultName);

			return dir.filePath (MakeCoverFileName (artist, album.Name_));
		}

		QString GetCoverPath (const QString& artist, const Collection::Album& album, const QDir& aaDir)
		{
			if (const auto curPath = GetCurrentCoverPath (album))
				return *curPath;

			if (XmlSettingsManager::Instance ().property ("PreferCoversStoreNearAlbums").toBool ())
				if (const auto inAlbumDir = GetAlbumDirCoverPath (album, artist))
					return *inAlbumDir;

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

		for (const auto& artist : std::as_const (artists))
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

		if (const QFileInfo fi (path);
			!failed && !(fi.isDir () && fi.isReadable ()))
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
