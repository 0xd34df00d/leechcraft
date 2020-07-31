/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "staticplaylistmanager.h"
#include <util/sys/paths.h>
#include "mediainfo.h"
#include "playlistparsers/m3u.h"

namespace LC
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

	void StaticPlaylistManager::SetOnLoadPlaylist (const NativePlaylist_t& sources)
	{
		WritePlaylist (GetOnLoadPath (), sources);
	}

	NativePlaylist_t StaticPlaylistManager::GetOnLoadPlaylist () const
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

	void StaticPlaylistManager::SaveCustomPlaylist (QString name, const NativePlaylist_t& sources)
	{
		WritePlaylist (PlaylistsDir_.filePath (GetFileName (name)), sources);
		emit customPlaylistsChanged ();
	}

	QStringList StaticPlaylistManager::EnumerateCustomPlaylists () const
	{
		static const QString plExt { ".m3u8" };

		auto result = PlaylistsDir_.entryList ({ "*" + plExt });
		for (auto& name : result)
			name.chop (plExt.size ());
		result.sort ();
		return result;
	}

	NativePlaylist_t StaticPlaylistManager::GetCustomPlaylist (const QString& name) const
	{
		return ReadPlaylist (GetCustomPlaylistPath (name));
	}

	QString StaticPlaylistManager::GetCustomPlaylistPath (const QString& name) const
	{
		return PlaylistsDir_.filePath (GetFileName (name));
	}

	void StaticPlaylistManager::DeleteCustomPlaylist (const QString& name)
	{
		if (PlaylistsDir_.remove (GetFileName (name)))
			emit customPlaylistsChanged ();
	}

	void StaticPlaylistManager::WritePlaylist (const QString& path, const NativePlaylist_t& sources)
	{
		M3U::Write (path, ToDumbPlaylist (sources));
	}

	NativePlaylist_t StaticPlaylistManager::ReadPlaylist (const QString& path) const
	{
		return FromDumbPlaylist (M3U::Read2Sources (path));
	}
}
}
