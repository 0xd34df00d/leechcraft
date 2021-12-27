/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumartmanager.h"
#include <functional>
#include <QTimer>
#include <QUrl>
#include <QtConcurrentRun>
#include <QFuture>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <util/network/handlenetworkreply.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/media/ialbumartprovider.h>
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

	QFuture<QList<QImage>> AlbumArtManager::CheckAlbumArt (const QString& artist, const QString& album)
	{
		if (Queue_.isEmpty ())
			ScheduleRotateQueue ();

		QFutureInterface<QList<QImage>> promise;
		Queue_.push_back ({ { artist, album }, promise });
		return promise.future ();
	}

	void AlbumArtManager::SetAlbumArt (int id, const QString& artist, const QString& album, const QImage& image)
	{
		auto joined = artist + "_-_" + album;
		joined.replace (' ', '_');
		const auto& filename = QUrl::toPercentEncoding (joined, QByteArray (), "~") + ".png";
		const auto& fullPath = AADir_.absoluteFilePath (filename);

		constexpr auto compressionRatio = 100;
		Util::Sequence (this, QtConcurrent::run ([image, fullPath] { image.save (fullPath, "PNG", compressionRatio); })) >>
				[this, id, fullPath] { Collection_.SetAlbumArt (id, fullPath); };
	}

	namespace
	{
		const auto NotFoundMarker = QStringLiteral ("NOTFOUND");
	}

	void AlbumArtManager::CheckNewArtists (const Collection::Artists_t& artists)
	{
		if (!XmlSettingsManager::Instance ().property ("AutoFetchAlbumArt").toBool ())
			return;

		for (const auto& artist : artists)
			for (const auto& album : artist.Albums_)
				if (album->CoverPath_.isEmpty () || album->CoverPath_ == NotFoundMarker)
					Util::Sequence (this, CheckAlbumArt (artist.Name_, album->Name_)) >>
							[ this
							, id = album->ID_
							, artist = artist.Name_
							, album = album->Name_
							] (const QList<QImage>& images)
							{
								if (images.isEmpty ())
									Collection_.SetAlbumArt (id, NotFoundMarker);
								else
									SetAlbumArt (id, artist, album,
											*std::max_element (images.begin (), images.end (), Util::ComparingBy (&QImage::width)));
							};
	}

	void AlbumArtManager::HandleGotUrls (const TaskQueue& task, const QList<QUrl>& urls)
	{
		if (urls.isEmpty ())
			return;

		const auto nam = GetProxyHolder ()->GetNetworkAccessManager ();

		QFutureInterface<QImage> promise;
		promise.reportStarted ();
		promise.setExpectedResultCount (urls.size ());

		auto decreateExpected = [promise] () mutable { promise.setExpectedResultCount (promise.expectedResultCount () - 1); };

		for (const auto& url : urls)
			Util::HandleReplySeq (nam->get (QNetworkRequest { url }), this) >>
					Util::Visitor
					{
						[=] (Util::Void) mutable { decreateExpected (); },
						[=] (const QByteArray& data) mutable
						{
							if (QImage image; image.loadFromData (data))
								promise.reportResult (image, promise.resultCount ());
							else
								decreateExpected ();
						}
					}.Finally ([=] () mutable
					{
						if (promise.resultCount () >= promise.expectedResultCount ())
							promise.reportFinished ();
					});

		auto watcher = new QFutureWatcher<QImage> { this };
		watcher->setFuture (promise.future ());
		connect (watcher,
				&QFutureWatcher<QImage>::finished,
				this,
				[watcher, task]
				{
					const auto& images = watcher->future ().results ();
					auto promise = task.Promise_;
					promise.reportFinished (&images);
				});
	}

	void AlbumArtManager::ScheduleRotateQueue ()
	{
		using namespace std::chrono_literals;
		QTimer::singleShot (500ms,
				this,
				&AlbumArtManager::RotateQueue);
	}

	void AlbumArtManager::RotateQueue ()
	{
		auto provs = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableRoots<Media::IAlbumArtProvider*> ();
		const auto& task = Queue_.takeFirst ();
		for (auto provObj : provs)
		{
			const auto prov = qobject_cast<Media::IAlbumArtProvider*> (provObj);
			const auto proxy = prov->RequestAlbumArt (task.Info_);
			Util::Sequence (this, prov->RequestAlbumArt (task.Info_)) >>
					Util::Visitor
					{
						[] (const QString&) {},
						[this, task] (const QList<QUrl>& urls) { HandleGotUrls (task, urls); }
					};
		}

		if (!Queue_.isEmpty ())
			ScheduleRotateQueue ();
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
			qWarning () << Q_FUNC_INFO
					<< "unable to create"
					<< path;

			const auto& e = Util::MakeNotification (QStringLiteral ("LMP"),
					tr ("Path %1 cannot be used as album art storage, default path will be used instead."),
					Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);

			AADir_ = Util::GetUserDir (Util::UserDir::Cache, QStringLiteral ("lmp/covers"));
		}
		else
			AADir_ = QDir (path);
	}
}
