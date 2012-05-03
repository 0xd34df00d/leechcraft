/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "albumartmanager.h"
#include <QTimer>
#include <QUrl>
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <util/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/ialbumartprovider.h>
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	AlbumArtManager::AlbumArtManager (QObject *parent)
	: QObject (parent)
	, AADir_ (Util::CreateIfNotExists ("lmp/covers"))
	{
	}

	void AlbumArtManager::CheckAlbumArt (const Collection::Artist& artist, Collection::Album_ptr album)
	{
		if (!album->CoverPath_.isEmpty () &&
				QPixmap (album->CoverPath_).isNull ())
			return;

		if (Queue_.isEmpty ())
			QTimer::singleShot (500,
					this,
					SLOT (rotateQueue ()));

		Queue_ << Media::AlbumInfo { artist.Name_, album->Name_ };
	}

	void AlbumArtManager::rotateQueue ()
	{
		auto provs = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<Media::IAlbumArtProvider*> ();
		Q_FOREACH (auto provObj, provs)
		{
			auto prov = qobject_cast<Media::IAlbumArtProvider*> (provObj);
			connect (provObj,
					SIGNAL (gotAlbumArt (Media::AlbumInfo, QList<QImage>)),
					this,
					SLOT (handleGotAlbumArt (Media::AlbumInfo, QList<QImage>)),
					Qt::UniqueConnection);
			prov->RequestAlbumArt (Queue_.takeFirst ());
		}

		if (!Queue_.isEmpty ())
			QTimer::singleShot (500,
					this,
					SLOT (rotateQueue ()));
	}

	void AlbumArtManager::handleGotAlbumArt (const Media::AlbumInfo& info, const QList<QImage>& images)
	{
		if (images.isEmpty ())
			return;

		auto image = images.first ();
		if (image.isNull ())
			return;

		auto collection = Core::Instance ().GetLocalCollection ();
		const auto id = collection->FindAlbum (info.Artist_, info.Album_);
		if (id < 0)
			return;

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

	void AlbumArtManager::handleSaved ()
	{
		const int id = sender ()->property ("ID").toInt ();
		const auto& fullPath = sender ()->property ("FullPath").toString ();
		Core::Instance ().GetLocalCollection ()->SetAlbumArt (id, fullPath);
		sender ()->deleteLater ();
	}
}
}
