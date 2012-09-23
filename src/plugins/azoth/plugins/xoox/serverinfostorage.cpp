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

#include "serverinfostorage.h"
#include <algorithm>
#include <QTimer>
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"
#include "accountsettingsholder.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	ServerInfoStorage::ServerInfoStorage (ClientConnection *conn, AccountSettingsHolder *settings)
	: QObject (conn)
	, Conn_ (conn)
	, Settings_ (settings)
	{
		connect (Conn_->GetClient (),
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));
	}

	bool ServerInfoStorage::HasServerFeatures () const
	{
		return !ServerFeatures_.isEmpty ();
	}

	QString ServerInfoStorage::GetBytestreamsProxy () const
	{
		return BytestreamsProxy_;
	}

	void ServerInfoStorage::HandleItems (const QXmppDiscoveryIq& iq)
	{
		Q_FOREACH (const auto& item, iq.items ())
			Conn_->RequestInfo (item.jid (),
					[this] (const QXmppDiscoveryIq& iq) { HandleItemInfo (iq); },
					false,
					item.node ());
	}

	void ServerInfoStorage::HandleServerInfo (const QXmppDiscoveryIq& iq)
	{
		ServerFeatures_ = iq.features ();
	}

	void ServerInfoStorage::HandleItemInfo (const QXmppDiscoveryIq& iq)
	{
		auto hasIdentity = [&iq] (const QString& cat, const QString& type)
		{
			const auto& ids = iq.identities ();
			return std::find_if (ids.begin (), ids.end (),
					[&cat, &type] (decltype (ids.front ()) id)
						{ return id.category () == cat && id.type () == type; }) != ids.end ();
		};
		if (hasIdentity ("proxy", "bytestreams"))
		{
			BytestreamsProxy_ = iq.from ();
			emit bytestreamsProxyChanged (BytestreamsProxy_);
		}
	}

	void ServerInfoStorage::handleConnected ()
	{
		if (Settings_->GetJID () == PreviousJID_)
			return;

		ServerFeatures_.clear ();

		BytestreamsProxy_.clear ();
		emit bytestreamsProxyChanged (QString ());

		PreviousJID_ = Settings_->GetJID ();
		Server_ = PreviousJID_.mid (PreviousJID_.indexOf ('@') + 1);
		if (Server_.isEmpty ())
			return;

		Conn_->RequestInfo (Server_,
				[this] (const QXmppDiscoveryIq& iq) { HandleServerInfo (iq); },
				false);
		Conn_->RequestItems (Server_,
				[this] (const QXmppDiscoveryIq& iq) { HandleItems (iq); },
				false);
	}
}
}
}
