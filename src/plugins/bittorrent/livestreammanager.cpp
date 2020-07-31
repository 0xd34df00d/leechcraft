/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "livestreammanager.h"
#include <interfaces/core/ientitymanager.h>
#include "livestreamdevice.h"

namespace LC
{
namespace BitTorrent
{
	LiveStreamManager::LiveStreamManager (CachedStatusKeeper *keeper,
			const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	, StatusKeeper_ { keeper }
	{
	}

	void LiveStreamManager::EnableOn (const libtorrent::torrent_handle& handle)
	{
		if (!Handle2Device_.contains (handle))
		{
			LiveStreamDevice *lsd = nullptr;
			try
			{
				lsd = new LiveStreamDevice { handle, StatusKeeper_, this };
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
				return;
			}

			Handle2Device_ [handle] = lsd;
			connect (lsd,
					SIGNAL (ready (LiveStreamDevice*)),
					this,
					SLOT (handleDeviceReady (LiveStreamDevice*)));
			lsd->CheckReady ();
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

	void LiveStreamManager::handleDeviceReady (LiveStreamDevice *lsd)
	{
		Entity e;
		e.Entity_ = QVariant::fromValue<QIODevice*> (lsd);
		e.Parameters_ = FromUserInitiated;
		e.Mime_ = "x-leechcraft/media-qiodevice";
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
