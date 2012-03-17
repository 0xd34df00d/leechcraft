/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "sdmanager.h"

namespace LeechCraft
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

	void SDManager::RequestInfo (ClientConnection::DiscoCallback_t callback,
			const QString& jid, const QString& node)
	{
		auto f = [this] (const QString& jid, ClientConnection::DiscoCallback_t cb, const QString& node)
				{ Conn_->RequestInfo (jid, cb, node); };
		CommonDo (Infos_, f, callback, jid, node);
	}

	void SDManager::RequestItems (ClientConnection::DiscoCallback_t callback,
			const QString& jid, const QString& node)
	{
		auto f = [this] (const QString& jid, ClientConnection::DiscoCallback_t cb, const QString& node)
				{ Conn_->RequestItems (jid, cb, node); };
		CommonDo (Items_, f, callback, jid, node);
	}

	void SDManager::CommonDo (SDManager::Cache_t& cache,
			std::function<void (const QString&, ClientConnection::DiscoCallback_t, const QString&)> f,
			ClientConnection::DiscoCallback_t cb,
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
