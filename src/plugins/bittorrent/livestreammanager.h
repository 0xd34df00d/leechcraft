/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>

namespace LC
{
namespace BitTorrent
{
	class LiveStreamDevice;
	class CachedStatusKeeper;

	class LiveStreamManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		CachedStatusKeeper * const StatusKeeper_;
		QMap<libtorrent::torrent_handle, LiveStreamDevice*> Handle2Device_;
	public:
		LiveStreamManager (CachedStatusKeeper*, const ICoreProxy_ptr&, QObject* = nullptr);

		void EnableOn (const libtorrent::torrent_handle&);
		bool IsEnabledOn (const libtorrent::torrent_handle&);
		void PieceRead (const libtorrent::read_piece_alert&);
	private slots:
		void handleDeviceReady (LiveStreamDevice*);
	};
}
}
