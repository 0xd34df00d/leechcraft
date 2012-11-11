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

#include "staticplaylistmanager.h"
#include <util/util.h>
#include "playlistparsers/m3u.h"

namespace LeechCraft
{
namespace LMP
{
	StaticPlaylistManager::StaticPlaylistManager (QObject *parent)
	: QObject (parent)
	, PlaylistsDir_ (Util::CreateIfNotExists ("lmp/playlists"))
	{
	}

	namespace
	{
		QString GetOnLoadPath ()
		{
			return Util::CreateIfNotExists ("lmp").filePath ("onload.m3u8");
		}
	}

	void StaticPlaylistManager::SetOnLoadPlaylist (const QList<Phonon::MediaSource>& sources)
	{
		WritePlaylist (GetOnLoadPath (), sources);
	}

	QList<Phonon::MediaSource> StaticPlaylistManager::GetOnLoadPlaylist () const
	{
		return ReadPlaylist (GetOnLoadPath ());
	}

	namespace
	{
		QString GetFileName (QString playlist)
		{
			playlist.remove ("../").remove ("./").remove ('/');
			return playlist + ".m3u8";
		}
	}

	void StaticPlaylistManager::SaveCustomPlaylist (QString name,
			const QList<Phonon::MediaSource>& sources)
	{
		WritePlaylist (PlaylistsDir_.filePath (GetFileName (name)), sources);
		emit customPlaylistsChanged ();
	}

	QStringList StaticPlaylistManager::EnumerateCustomPlaylists () const
	{
		QStringList result = PlaylistsDir_.entryList (QStringList ("*.m3u8"));
		for (auto i = result.begin (), end = result.end (); i != end; ++i)
			i->chop (5);
		result.sort ();
		return result;
	}

	QList<Phonon::MediaSource> StaticPlaylistManager::GetCustomPlaylist (const QString& name) const
	{
		return ReadPlaylist (PlaylistsDir_.filePath (GetFileName (name)));
	}

	void StaticPlaylistManager::DeleteCustomPlaylist (const QString& name)
	{
		if (PlaylistsDir_.remove (GetFileName (name)))
			emit customPlaylistsChanged ();
	}

	void StaticPlaylistManager::WritePlaylist (const QString& path, const QList<Phonon::MediaSource>& sources)
	{
		M3U::Write (path, sources);
	}

	QList<Phonon::MediaSource> StaticPlaylistManager::ReadPlaylist (const QString& path) const
	{
		return M3U::Read2Sources (path);
	}
}
}
