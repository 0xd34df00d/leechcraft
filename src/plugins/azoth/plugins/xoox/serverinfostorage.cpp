/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverinfostorage.h"
#include <algorithm>
#include <QTimer>
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"
#include "accountsettingsholder.h"
#include "discomanagerwrapper.h"

namespace LC
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

	QStringList ServerInfoStorage::GetServerFeatures () const
	{
		return ServerFeatures_;
	}

	bool ServerInfoStorage::HasSelfFeatures () const
	{
		return !SelfJIDFeatures_.isEmpty ();
	}

	QStringList ServerInfoStorage::GetSelfFeatures () const
	{
		return SelfJIDFeatures_;
	}

	QString ServerInfoStorage::GetBytestreamsProxy () const
	{
		return BytestreamsProxy_;
	}

	void ServerInfoStorage::HandleItems (const QXmppDiscoveryIq& iq)
	{
		for (const auto& item : iq.items ())
			Conn_->GetDiscoManagerWrapper ()->RequestInfo (item.jid (),
					[this] (const QXmppDiscoveryIq& iq) { HandleItemInfo (iq); },
					false,
					item.node ());
	}

	void ServerInfoStorage::HandleItemInfo (const QXmppDiscoveryIq& iq)
	{
		auto hasIdentity = [&iq] (const QString& cat, const QString& type) -> bool
		{
			const auto& ids = iq.identities ();
			return std::any_of (ids.begin (), ids.end (),
					[&cat, &type] (const auto& id) { return id.category () == cat && id.type () == type; });
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

		Conn_->GetDiscoManagerWrapper ()->RequestInfo (Server_,
				[this] (const QXmppDiscoveryIq& iq) { ServerFeatures_ = iq.features (); },
				false);
		Conn_->GetDiscoManagerWrapper ()->RequestItems (Server_,
				[this] (const QXmppDiscoveryIq& iq) { HandleItems (iq); },
				false);
		Conn_->GetDiscoManagerWrapper ()->RequestInfo (Settings_->GetJID (),
				[this] (const QXmppDiscoveryIq& iq) { SelfJIDFeatures_ = iq.features (); },
				false);
	}
}
}
}
