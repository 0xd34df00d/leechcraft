/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMap>

namespace libtorrent
{
	class torrent_handle;
};

namespace LC::BitTorrent
{
	class AlertDispatcher;
	class CachedStatusKeeper;
	class LiveStreamDevice;

	class LiveStreamManager : public QObject
	{
		CachedStatusKeeper& StatusKeeper_;
		QMap<libtorrent::torrent_handle, std::shared_ptr<LiveStreamDevice>> Handle2Device_;
	public:
		explicit LiveStreamManager (CachedStatusKeeper&, AlertDispatcher&, QObject* = nullptr);

		void EnableOn (const libtorrent::torrent_handle&);
		bool IsEnabledOn (const libtorrent::torrent_handle&);
	};
}
