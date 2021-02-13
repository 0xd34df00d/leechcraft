/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/session_status.hpp>

namespace LC::BitTorrent
{
	struct TorrentInfo
	{
		QString Destination_;
		QString State_;
		libtorrent::torrent_status Status_;
		std::optional<libtorrent::torrent_info> Info_;
	};
}

Q_DECLARE_METATYPE (LC::BitTorrent::TorrentInfo)
