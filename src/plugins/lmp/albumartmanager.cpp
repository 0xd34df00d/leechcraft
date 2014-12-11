/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "albumartmanager.h"
#include <functional>
#include <QTimer>
#include <QUrl>
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/media/ialbumartprovider.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace LMP
{
	AlbumArtManager::AlbumArtManager (QObject *parent)
	: QObject (parent)
	{
		XmlSettingsManager::Instance ().RegisterObject ("CoversStoragePath",
				this, "handleCoversPath");

		QTimer::singleShot (0,
				this,
				SLOT (handleCoversPath ()));
	}

	void AlbumArtManager::CheckAlbumArt (const Collection::Artist& artist, Collection::Album_ptr album)
	{
		const auto& path = album->CoverPath_;
		if (!path.isEmpty () && QFile::exists (path))
			return;

		CheckAlbumArt (artist.Name_, album->Name_, false);
	}

	void AlbumArtManager::CheckAlbumArt (const QString& artist, const QString& album, bool preview)
	{
		if (Queue_.isEmpty ())
			QTimer::singleShot (500,
					this,
					SLOT (rotateQueue ()));
		Queue_.push_back ({ { artist, album }, preview });
	}

	void AlbumArtManager::handleGotAlbumArt (const Media::AlbumInfo& info, const QList<QImage>& images)
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

		auto imgCmp = [] (const QImage& l, const QImage& r) { return l.width () < r.width (); };

		const auto& image = images.size () > 1 ?
				*std::max_element (images.begin (), images.end (), imgCmp) :
				images.value (0);

		if (BestSizes_.value (info).width () >= image.width ())
			return;

		BestSizes_ [info] = image.size ();

		if (image.isNull ())
		{
			if (!NumRequests_ [info])
				collection->SetAlbumArt (id, "NOTFOUND");
			return;
		}

		auto joined = info.Artist_ + "_-_" + info.Album_;
		joined.replace (' ', '_');
		const auto& filename = QUrl::toPercentEncoding (joined, QByteArray (), "~") + ".png";
		const auto& fullPath = AADir_.absoluteFilePath (filename);

		auto watcher = new QFutureWatcher<void> ();
		watcher->setProperty ("ID", id);
		watcher->setProperty ("FullPath", fullPath);
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleSaved ()));
		watcher->setFuture (QtConcurrent::run ([image, fullPath] () { image.save (fullPath, "PNG", 100); }));
	}

	void AlbumArtManager::rotateQueue ()
	{
		auto provs = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<Media::IAlbumArtProvider*> ();
		const auto& task = Queue_.takeFirst ();
		for (auto provObj : provs)
		{
			const auto prov = qobject_cast<Media::IAlbumArtProvider*> (provObj);
			const auto proxy = prov->RequestAlbumArt (task.Info_);
			connect (proxy->GetQObject (),
					SIGNAL (ready (Media::AlbumInfo, QList<QImage>)),
					this,
					task.PreviewMode_ ?
							SIGNAL (gotImages (Media::AlbumInfo, QList<QImage>)) :
							SLOT (handleGotAlbumArt (Media::AlbumInfo, QList<QImage>)));
		}
		if (!provs.isEmpty ())
			NumRequests_ [task.Info_] = provs.size ();

		if (!Queue_.isEmpty ())
			QTimer::singleShot (500,
					this,
					SLOT (rotateQueue ()));
	}

	void AlbumArtManager::handleSaved ()
	{
		const int id = sender ()->property ("ID").toInt ();
		const auto& fullPath = sender ()->property ("FullPath").toString ();
		Core::Instance ().GetLocalCollection ()->SetAlbumArt (id, fullPath);
		sender ()->deleteLater ();
	}

	void AlbumArtManager::handleCoversPath ()
	{
		const auto& path = XmlSettingsManager::Instance ()
				.property ("CoversStoragePath").toString ();

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

			const auto& e = Util::MakeNotification ("LMP",
					tr ("Path %1 cannot be used as album art storage, default path will be used instead."),
					PWarning_);
			Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);

			AADir_ = Util::GetUserDir (Util::UserDir::Cache, "lmp/covers");
		}
		else
			AADir_ = QDir (path);
	}
}
}
