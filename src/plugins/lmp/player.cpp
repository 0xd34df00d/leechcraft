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

#include "player.h"
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include "core.h"
#include "mediainfo.h"
#include "localfileresolver.h"

Q_DECLARE_METATYPE (Phonon::MediaSource);

namespace Phonon
{
	uint qHash (const Phonon::MediaSource& src)
	{
		uint hash = 0;
		switch (src.type ())
		{
		case Phonon::MediaSource::LocalFile:
			hash = qHash (src.fileName ());
			break;
		case Phonon::MediaSource::Url:
			hash = qHash (src.url ());
			break;
		case Phonon::MediaSource::Disc:
			hash = src.discType ();
			break;
		case Phonon::MediaSource::Stream:
			hash = qHash (src.deviceName ());
			break;
		default:
			hash = 0;
			break;
		}
		return hash << src.type ();
	}
}

namespace LeechCraft
{
namespace LMP
{
	Player::Player (QObject *parent)
	: QObject (parent)
	, PlaylistModel_ (new QStandardItemModel (this))
	, Source_ (new Phonon::MediaObject (this))
	, Path_ (Phonon::createPath (Source_, new Phonon::AudioOutput (Phonon::MusicCategory, this)))
	{
		connect (Source_,
				SIGNAL (currentSourceChanged (Phonon::MediaSource)),
				this,
				SLOT (handleCurrentSourceChanged (Phonon::MediaSource)));
	}

	QAbstractItemModel* Player::GetPlaylistModel () const
	{
		return PlaylistModel_;
	}

	void Player::Enqueue (const QList<Phonon::MediaSource>& sources)
	{
		AddToPlaylistModel (sources);

		const auto& current = Source_->currentSource ();
		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
		if (pos == CurrentQueue_.end ())
			pos = CurrentQueue_.begin ();

		QList<Phonon::MediaSource> toPath;
		std::copy (pos, CurrentQueue_.end (), std::back_inserter (toPath));
		Source_->enqueue (toPath);
	}

	namespace
	{
		void FillItem (QStandardItem *item, const MediaInfo& info)
		{
			item->setText (QString ("%1 - %2 - %3").arg (info.Artist_).arg (info.Album_).arg (info.Title_));
			item->setData (QVariant::fromValue (info), Player::Role::MediaInfo);
		}

		QPixmap FindAlbumArt (const QString& near)
		{
			QStringList possibleBases;
			possibleBases << "cover" << "folder" << "front";

			const QDir& dir = QFileInfo (near).absoluteDir ();
			const QStringList& entryList = dir.entryList (QStringList ("*.jpg") << "*.png");
			auto pos = std::find_if (entryList.begin (), entryList.end (),
					[&possibleBases] (const QString& name)
					{
						Q_FOREACH (const QString& pBase, possibleBases)
							if (name.startsWith (pBase, Qt::CaseInsensitive))
								return true;
						return false;
					});
			if (pos == entryList.end ())
				return QPixmap ();

			return QPixmap (dir.filePath (*pos));
		}

		QStandardItem* MakeAlbumItem (const MediaInfo& info)
		{
			auto albumItem = new QStandardItem (QString ("%1 - %2")
							.arg (info.Artist_, info.Album_));
			albumItem->setEditable (false);
			albumItem->setData (true, Player::Role::IsAlbum);
			albumItem->setData (QVariant::fromValue (info), Player::Role::MediaInfo);
			albumItem->setData (FindAlbumArt (info.LocalPath_), Player::Role::AlbumArt);
			albumItem->setData (0, Player::Role::AlbumLength);
			return albumItem;
		}

		void IncAlbumLength (QStandardItem *albumItem, int length)
		{
			const int prevLength = albumItem->data (Player::Role::AlbumLength).toInt ();
			albumItem->setData (length + prevLength, Player::Role::AlbumLength);
		}
	}

	void Player::AddToPlaylistModel (QList<Phonon::MediaSource> sources)
	{
		if (!CurrentQueue_.isEmpty ())
		{
			PlaylistModel_->clear ();
			Items_.clear ();
			AlbumRoots_.clear ();

			auto newList = CurrentQueue_ + sources;
			CurrentQueue_.clear ();
			AddToPlaylistModel (newList);
			return;
		}

		ApplyOrdering (sources);
		CurrentQueue_ = sources;

		auto resolver = Core::Instance ().GetLocalFileResolver ();

		Q_FOREACH (const auto& source, sources)
		{
			auto item = new QStandardItem ();
			item->setEditable (false);
			item->setData (QVariant::fromValue (source), Role::MediaSource);
			switch (source.type ())
			{
			case Phonon::MediaSource::Stream:
				item->setText (tr ("Stream"));
				PlaylistModel_->appendRow (item);
				break;
			case Phonon::MediaSource::Url:
				item->setText ("URL");
				PlaylistModel_->appendRow (item);
				break;
			case Phonon::MediaSource::LocalFile:
			{
				const auto& info = resolver->ResolveInfo (source.fileName ());
				const auto& albumID = qMakePair (info.Artist_, info.Album_);
				FillItem (item, info);
				if (!AlbumRoots_.contains (albumID))
				{
					PlaylistModel_->appendRow (item);
					AlbumRoots_ [albumID] = item;
				}
				else if (AlbumRoots_ [albumID]->data (Role::IsAlbum).toBool ())
				{
					IncAlbumLength (AlbumRoots_ [albumID], info.Length_);
					AlbumRoots_ [albumID]->appendRow (item);
				}
				else
				{
					auto albumItem = MakeAlbumItem (info);

					const int row = AlbumRoots_ [albumID]->row ();
					const auto& existing = PlaylistModel_->takeRow (row);
					albumItem->appendRow (existing);
					albumItem->appendRow (item);
					PlaylistModel_->insertRow (row, albumItem);

					const auto& existingInfo = existing.at (0)->data (Role::MediaInfo).value<LMP::MediaInfo> ();
					albumItem->setData (existingInfo.Length_, Role::AlbumLength);
					IncAlbumLength (albumItem, info.Length_);

					emit insertedAlbum (albumItem->index ());

					AlbumRoots_ [albumID] = albumItem;
				}
				break;
			}
			default:
				item->setText ("unknown");
				PlaylistModel_->appendRow (item);
				break;
			}

			Items_ [source] = item;
		}
	}

	void Player::ApplyOrdering (QList<Phonon::MediaSource>& sources)
	{
		auto resolver = Core::Instance ().GetLocalFileResolver ();
		std::sort (sources.begin (), sources.end (),
				[resolver] (const Phonon::MediaSource& s1, const Phonon::MediaSource& s2)
				{
					qDebug () << s1.type () << s2.type ();
					if (s1.type () != Phonon::MediaSource::LocalFile ||
						s2.type () != Phonon::MediaSource::LocalFile)
						return qHash (s1) < qHash (s2);

					const auto& left = resolver->ResolveInfo (s1.fileName ());
					const auto& right = resolver->ResolveInfo (s2.fileName ());
					if (left.Artist_ != right.Artist_)
						return left.Artist_ < right.Artist_;
					if (left.Year_ != right.Year_)
						return left.Year_ < right.Year_;
					if (left.Album_ != right.Album_)
						return left.Album_ < right.Album_;
					if (left.TrackNumber_ != right.TrackNumber_)
						return left.TrackNumber_ < right.TrackNumber_;
					return left.Title_ < right.Title_;
				});
	}

	void Player::play (const QModelIndex& index)
	{
		if (index.data (Role::IsAlbum).toBool ())
		{
			play (index.child (0, 0));
			return;
		}

		if (!index.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index;
			return;
		}

		Source_->stop ();
		const auto& source = index.data (Role::MediaSource).value<Phonon::MediaSource> ();
		Source_->setCurrentSource (source);
		Source_->play ();
	}

	void Player::clear ()
	{
		PlaylistModel_->clear ();
		Items_.clear ();
		AlbumRoots_.clear ();
		Source_->clearQueue ();
	}

	void Player::handleCurrentSourceChanged (const Phonon::MediaSource& source)
	{
		auto curItem = Items_ [source];
		curItem->setData (true, Role::IsCurrent);

		Q_FOREACH (auto item, Items_.values ())
		{
			if (item == curItem)
				continue;
			if (item->data (Role::IsCurrent).toBool ())
			{
				item->setData (false, Role::IsCurrent);
				break;
			}
		}
	}
}
}
