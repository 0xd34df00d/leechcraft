/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QTime>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/session_status.hpp>

namespace LC
{
namespace BitTorrent
{
	struct TorrentInfo
	{
		QString Destination_,
				State_;
		libtorrent::torrent_status Status_;
		std::unique_ptr<libtorrent::torrent_info> Info_;
	};
}
}
