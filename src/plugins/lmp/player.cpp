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

	namespace
	{
		void FillItem (QStandardItem *item, const MediaInfo& info)
		{
			item->setText (QString ("%1 - %2 - %3").arg (info.Artist_).arg (info.Album_).arg (info.Title_));
			item->setData (QVariant::fromValue (info), Player::Role::MediaInfo);
		}
	}

	void Player::Enqueue (const QList<Phonon::MediaSource>& sources)
	{
		Source_->enqueue (sources);

		Q_FOREACH (const auto& source, sources)
		{
			auto item = new QStandardItem ();
			item->setEditable (false);
			item->setData (QVariant::fromValue (source), Role::MediaSource);
			switch (source.type ())
			{
			case Phonon::MediaSource::Stream:
				item->setText (tr ("Stream"));
				break;
			case Phonon::MediaSource::Url:
				item->setText ("URL");
				break;
			case Phonon::MediaSource::LocalFile:
				FillItem (item, Core::Instance ().GetLocalFileResolver ()->
							ResolveInfo (source.fileName ()));
				break;
			default:
				item->setText ("unknown");
				break;
			}
			PlaylistModel_->appendRow (item);

			Items_ [source] = item;
		}
	}

	void Player::play (const QModelIndex& index)
	{
		Source_->stop ();
		const auto& source = index.data (Role::MediaSource).value<Phonon::MediaSource> ();
		Source_->setCurrentSource (source);
		Source_->play ();
	}

	void Player::clear ()
	{
		PlaylistModel_->clear ();
		Items_.clear ();
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
