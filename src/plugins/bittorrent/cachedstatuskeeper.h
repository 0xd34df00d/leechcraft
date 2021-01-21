/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

namespace LC::BitTorrent
{
	class CachedStatusKeeper : public QObject
	{
	public:
		using FlagsType_t = libtorrent::status_flags_t;
		constexpr static FlagsType_t AllFlags = FlagsType_t::all ();
	private:
		struct CachedItem
		{
			libtorrent::torrent_status Status_;
			FlagsType_t ReqFlags_;
		};

		QMap<libtorrent::torrent_handle, CachedItem> Handle2Status_;
	public:
		using QObject::QObject;

		libtorrent::torrent_status GetStatus (const libtorrent::torrent_handle&, FlagsType_t flags = {});
		void HandleStatusUpdatePosted (const libtorrent::torrent_status&);
	};
}
