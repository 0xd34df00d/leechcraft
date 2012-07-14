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

#pragma once

#include <QObject>
#include <QHash>
#include <QXmppDiscoveryIq.h>
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class SDManager : public QObject
	{
		Q_OBJECT

		ClientConnection *Conn_;

		typedef QHash<QString, QHash<QString, QXmppDiscoveryIq>> Cache_t;
		Cache_t Infos_;
		Cache_t Items_;
	public:
		SDManager (ClientConnection*);

		void RequestInfo (ClientConnection::DiscoCallback_t callback,
				const QString& jid, const QString& node = QString ());
		void RequestItems (ClientConnection::DiscoCallback_t callback,
				const QString& jid, const QString& node = QString ());
	private:
		void CommonDo (Cache_t& cache,
				std::function<void (const QString&, ClientConnection::DiscoCallback_t, const QString&)>,
				ClientConnection::DiscoCallback_t,
				const QString&, const QString&);
	};
}
}
}
