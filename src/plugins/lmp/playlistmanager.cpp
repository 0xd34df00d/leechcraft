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

#include "playlistmanager.h"
#include <QStandardItemModel>
#include <QTimer>
#include "core.h"
#include "staticplaylistmanager.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	PlaylistManager::PlaylistManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	, StaticRoot_ (new QStandardItem (tr ("Static playlists")))
	, Static_ (new StaticPlaylistManager (this))
	{
		StaticRoot_->setEditable (false);
		Model_->appendRow (StaticRoot_);

		connect (Static_,
				SIGNAL (customPlaylistsChanged ()),
				this,
				SLOT (handleStaticPlaylistsChanged ()));
		QTimer::singleShot (100,
				this,
				SLOT (handleStaticPlaylistsChanged ()));

		auto dynamicRoot = new QStandardItem (tr ("Dynamic playlists"));
		dynamicRoot->setEditable (false);
		Model_->appendRow (dynamicRoot);

		const std::vector<PlaylistTypes> types = { PlaylistTypes::Random50 };
		const std::vector<QString> names = { tr ("50 random tracks") };
		for (size_t i = 0, size = types.size (); i < size; ++i)
		{
			auto item = new QStandardItem (names.at (i));
			item->setData (types.at (i), Roles::PlaylistType);
			item->setEditable (false);
			dynamicRoot->appendRow (item);
		}
	}

	QAbstractItemModel* PlaylistManager::GetPlaylistsModel () const
	{
		return Model_;
	}

	StaticPlaylistManager* PlaylistManager::GetStaticManager () const
	{
		return Static_;
	}

	QList<Phonon::MediaSource> PlaylistManager::GetSources (const QModelIndex& index) const
	{
		auto col = Core::Instance ().GetLocalCollection ();
		auto toSrcs = [col] (const QList<int>& ids)
		{
			const auto& paths = col->TrackList2PathList (ids);
			QList<Phonon::MediaSource> result;
			std::transform (paths.begin (), paths.end (), std::back_inserter (result),
					[] (const QString& path) { return Phonon::MediaSource (path); });
			return result;
		};

		switch (index.data (Roles::PlaylistType).toInt ())
		{
		case PlaylistTypes::Static:
			return Static_->GetCustomPlaylist (index.data ().toString ());
		case PlaylistTypes::Random50:
			return toSrcs (col->GetDynamicPlaylist (LocalCollection::DynamicPlaylist::Random50));
		default:
			return QList<Phonon::MediaSource> ();
		}
	}

	void PlaylistManager::handleStaticPlaylistsChanged ()
	{
		while (StaticRoot_->rowCount ())
			StaticRoot_->removeRow (0);

		const auto& icon = Core::Instance ().GetProxy ()->GetIcon ("view-media-playlist");
		Q_FOREACH (const auto& name, Static_->EnumerateCustomPlaylists ())
		{
			auto item = new QStandardItem (icon, name);
			item->setData (PlaylistTypes::Static, Roles::PlaylistType);
			item->setEditable (false);
			StaticRoot_->appendRow (item);
		}
	}
}
}
