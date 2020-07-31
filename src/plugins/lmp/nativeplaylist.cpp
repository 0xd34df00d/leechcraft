/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nativeplaylist.h"
#include <util/sll/prelude.h>
#include "mediainfo.h"
#include "playlistparsers/playlist.h"

namespace LC
{
namespace LMP
{
	Playlist ToDumbPlaylist (const NativePlaylist_t& playlist)
	{
		return Util::Map (playlist,
				[] (const NativePlaylistItem_t& item)
				{
					return item.second ?
							PlaylistItem { item.first, *item.second } :
							PlaylistItem { item.first };
				});
	}

	NativePlaylist_t FromDumbPlaylist (const Playlist& playlist)
	{
		return Util::Map (playlist,
				[] (const PlaylistItem& item)
				{
					return NativePlaylistItem_t { item.Source_, item.GetMediaInfo () };
				});
	}
}
}
