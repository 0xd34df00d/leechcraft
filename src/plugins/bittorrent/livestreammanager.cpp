/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "livestreammanager.h"
#include <QtDebug>
#include <libtorrent/alert_types.hpp>
#include "livestreamdevice.h"

namespace LC::BitTorrent
{
	LiveStreamManager::LiveStreamManager (CachedStatusKeeper *keeper, QObject *parent)
	: QObject { parent }
	, StatusKeeper_ { keeper }
	{
	}

	void LiveStreamManager::EnableOn (const libtorrent::torrent_handle& handle)
	{
		if (Handle2Device_.contains (handle))
			return;

		try
		{
			auto lsd = std::make_shared<LiveStreamDevice> (handle, StatusKeeper_);
			lsd->CheckReady ();

			Handle2Device_ [handle] = std::move (lsd);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}

	bool LiveStreamManager::IsEnabledOn (const libtorrent::torrent_handle& handle)
	{
		return Handle2Device_.contains (handle);
	}

	void LiveStreamManager::PieceRead (const libtorrent::read_piece_alert& a)
	{
		const auto& handle = a.handle;

		if (!Handle2Device_.contains (handle))
		{
			qWarning () << Q_FUNC_INFO
					<< "Handle2Device_ doesn't contain handle"
					<< Handle2Device_.size ();
			return;
		}

		Handle2Device_ [handle]->PieceRead (a);
	}
}
