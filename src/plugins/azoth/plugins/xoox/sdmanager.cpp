/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sdmanager.h"
#include "clientconnection.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	SDManager::SDManager (ClientConnection *conn)
	: QObject (conn)
	, Conn_ (conn)
	{
	}

	void SDManager::RequestInfo (DiscoManagerWrapper::DiscoCallback_t callback,
			const QString& jid, const QString& node)
	{
		auto f = [this] (const QString& jid, DiscoManagerWrapper::DiscoCallback_t cb, const QString& node)
				{ Conn_->GetDiscoManagerWrapper ()->RequestInfo (jid, cb, false, node); };
		CommonDo (Infos_, f, callback, jid, node);
	}

	void SDManager::RequestItems (DiscoManagerWrapper::DiscoCallback_t callback,
			const QString& jid, const QString& node)
	{
		auto f = [this] (const QString& jid, DiscoManagerWrapper::DiscoCallback_t cb, const QString& node)
				{ Conn_->GetDiscoManagerWrapper ()->RequestItems (jid, cb, false, node); };
		CommonDo (Items_, f, callback, jid, node);
	}

	void SDManager::CommonDo (SDManager::Cache_t& cache,
			std::function<void (const QString&, DiscoManagerWrapper::DiscoCallback_t, const QString&)> f,
			DiscoManagerWrapper::DiscoCallback_t cb,
			const QString& jid, const QString& node)
	{
		if (cache [jid].contains (node))
		{
			cb (cache [jid] [node]);
			return;
		}

		auto ourCb = [cb, &cache] (const QXmppDiscoveryIq& iq)
		{
			cache [iq.from ()] [iq.queryNode ()] = iq;
			cb (iq);
		};
		f (jid, ourCb, node);
	}
}
}
}
