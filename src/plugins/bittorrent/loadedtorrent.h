/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <libtorrent/add_torrent_params.hpp>
#include "torrentcreationinfo.h"

class QByteArray;

namespace LC::BitTorrent
{
	struct LoadedTorrent
	{
		libtorrent::add_torrent_params Params_;
		TorrentCreationInfo CreationInfo_;

		static LoadedTorrent Load (const QByteArray& torrent, const QByteArray& resumeData = {});
	};
}
