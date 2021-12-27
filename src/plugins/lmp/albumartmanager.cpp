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
#include <QFutureSynchronizer>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <util/network/handlenetworkreply.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/media/ialbumartprovider.h>
#include "core.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"

namespace LC::LMP
{
	AlbumArtManager::AlbumArtManager (QObject *parent)
	: QObject { parent }
	{
		XmlSettingsManager::Instance ().RegisterObject ("CoversStoragePath", this,
				[this] (const QVariant& var) { HandleCoversPath (var.toString ()); });
	}

	void AlbumArtManager::CheckAlbumArt (const QString& artist, const QString& album, bool preview)
	{
		if (Queue_.isEmpty ())
			ScheduleRotateQueue ();

		Queue_.push_back ({ { artist, album }, preview });
	}

	void AlbumArtManager::HandleGotAlbumArt (const Media::AlbumInfo& info, const QList<QImage>& images)
	{
		auto collection = Core::Instance ().GetLocalCollection ();
		const auto id = collection->FindAlbum (info.Artist_, info.Album_);
		if (id < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "album not found"
					<< info.Artist_
					<< info.Album_;
			return;
		}

		--NumRequests_ [info];

		const auto& image = images.size () > 1 ?
				*std::max_element (images.begin (), images.end (), Util::ComparingBy (&QImage::width)) :
				images.value (0);

		if (BestSizes_.value (info).width () >= image.width ())
			return;

		BestSizes_ [info] = image.size ();

		if (image.isNull ())
		{
			if (!NumRequests_ [info])
				collection->SetAlbumArt (id, QStringLiteral ("NOTFOUND"));
			return;
		}

		auto joined = info.Artist_ + "_-_" + info.Album_;
		joined.replace (' ', '_');
		const auto& filename = QUrl::toPercentEncoding (joined, QByteArray (), "~") + ".png";
		const auto& fullPath = AADir_.absoluteFilePath (filename);

		constexpr auto compressionRatio = 100;
		Util::Sequence (this, QtConcurrent::run ([image, fullPath] { image.save (fullPath, "PNG", compressionRatio); })) >>
				[id, fullPath] { Core::Instance ().GetLocalCollection ()->SetAlbumArt (id, fullPath); };
	}

	void AlbumArtManager::HandleGotUrls (const TaskQueue& task, const QList<QUrl>& urls)
	{
		using MaybeImage_t = std::optional<QImage>;

		const auto nam = GetProxyHolder ()->GetNetworkAccessManager ();

		auto sync = std::make_shared<QFutureSynchronizer<MaybeImage_t>> ();
		for (const auto& url : urls)
		{
			QFutureInterface<MaybeImage_t> promise;
			promise.reportStarted ();

			Util::HandleReplySeq (nam->get (QNetworkRequest { url }), this) >>
					Util::Visitor
					{
						[promise] (Util::Void) mutable { Util::ReportFutureResult (promise, MaybeImage_t {}); },
						[promise] (const QByteArray& data) mutable
						{
							if (QImage image; image.loadFromData (data))
								Util::ReportFutureResult (promise, image);
							else
								Util::ReportFutureResult (promise, MaybeImage_t {});
						}
					};

			sync->addFuture (promise.future ());
		}

		const auto& future = QtConcurrent::run ([sync]
				{
					sync->waitForFinished ();
					return Util::Map (sync->futures (), &QFuture<MaybeImage_t>::result);
				});
		Util::Sequence (this, future) >>
				[this, task] (const QList<MaybeImage_t>& maybeImages)
				{
					QList<QImage> images;
					for (const auto& maybe : maybeImages)
						if (maybe)
							images << *maybe;

					if (task.PreviewMode_)
						emit gotImages (task.Info_, images);
					else
						HandleGotAlbumArt (task.Info_, images);
				};
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
		if (!provs.isEmpty ())
			NumRequests_ [task.Info_] = provs.size ();

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
