/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/optional.hpp>
#include "engine/audiosource.h"

namespace LC
{
namespace LMP
{
	class Playlist;
	struct MediaInfo;

	using NativePlaylistItem_t = QPair<AudioSource, boost::optional<MediaInfo>>;
	using NativePlaylist_t = QList<NativePlaylistItem_t>;

	Playlist ToDumbPlaylist (const NativePlaylist_t&);
	NativePlaylist_t FromDumbPlaylist (const Playlist&);
}
}
