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

#pragma once

#include <QObject>
#include <phonon/mediasource.h>
#include <phonon/path.h>

class QModelIndex;
class QStandardItem;
class QAbstractItemModel;
class QStandardItemModel;

namespace Phonon
{
	class MediaObject;
}

namespace LeechCraft
{
namespace LMP
{
	struct MediaInfo;

	class Player : public QObject
	{
		Q_OBJECT

		QStandardItemModel *PlaylistModel_;
		Phonon::MediaObject *Source_;
		Phonon::Path Path_;

		QList<Phonon::MediaSource> CurrentQueue_;
		QHash<Phonon::MediaSource, QStandardItem*> Items_;
		QHash<QPair<QString, QString>, QStandardItem*> AlbumRoots_;
	public:
		enum class PlayMode
		{
			Sequential,
			Shuffle,
			RepeatTrack,
			RepeatAlbum,
			RepeatWhole
		};
	private:
		PlayMode PlayMode_;
	public:
		enum Role
		{
			IsCurrent = Qt::UserRole + 1,
			IsAlbum,
			Source,
			Info,
			AlbumArt,
			AlbumLength
		};

		Player (QObject* = 0);

		QAbstractItemModel* GetPlaylistModel () const;
		Phonon::MediaObject* GetSourceObject () const;

		void SetPlayMode (PlayMode);

		void Enqueue (const QStringList&);
		void Enqueue (const QList<Phonon::MediaSource>&);
	private:
		MediaInfo GetMediaInfo (const Phonon::MediaSource&) const;
		void AddToPlaylistModel (QList<Phonon::MediaSource>);
		void ApplyOrdering (QList<Phonon::MediaSource>&);
	public slots:
		void play (const QModelIndex&);
		void previousTrack ();
		void nextTrack ();
		void togglePause ();
		void stop ();
		void clear ();
	private slots:
		void handleSourceAboutToFinish ();
		void handleCurrentSourceChanged (const Phonon::MediaSource&);
	signals:
		void songChanged (const MediaInfo&);
		void insertedAlbum (const QModelIndex&);
	};
}
}
