/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "capsmanager.h"
#include "clientconnection.h"
#include "capsdatabase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	CapsManager::CapsManager (ClientConnection *connection)
	: QObject (connection)
	, Connection_ (connection)
	, DB_ (new CapsDatabase (this))
	{
		Caps2String_ ["http://etherx.jabber.org/streams"] = "stream";
		Caps2String_ ["jabber:client"] = "client";
		Caps2String_ ["jabber:server"] = "server";
		Caps2String_ ["jabber:server:dialback"] = "XEP-0220: Server Dialback";
		Caps2String_ ["jabber:iq:roster"] = "Jabber Roster";
		Caps2String_ ["urn:ietf:params:xml:ns:xmpp-tls"] = "TLS";
		Caps2String_ ["urn:ietf:params:xml:ns:xmpp-sasl"] = "SASL";
		Caps2String_ ["urn:ietf:params:xml:ns:xmpp-bind"] = "Bind";
		Caps2String_ ["urn:ietf:params:xml:ns:xmpp-session"] = "session";
		Caps2String_ ["urn:ietf:params:xml:ns:xmpp-stanzas"] = "stanzas";
		Caps2String_ ["vcard-temp"] = "XEP-0054: vCards";
		Caps2String_ ["vcard-temp:x:update"] = "XEP-0054: vCard update";
		Caps2String_ ["jabber:iq:auth"] = "XEP-0078: non-SASL authentication";
		Caps2String_ ["http://jabber.org/features/iq-auth"] = "XEP-0078: non-SASL authentication (features)";
		Caps2String_ ["http://jabber.org/protocol/caps"] = "XEP-0115: Entity Capabilities";
		Caps2String_ ["http://jabber.org/protocol/compress"] = "XEP-0138: Stream Compression";
		Caps2String_ ["http://jabber.org/features/compress"] = "XEP-0138: Stream Compression (features)";
		Caps2String_ ["http://jabber.org/protocol/disco#info"] = "XEP-0030: Service Discovery (info)";
		Caps2String_ ["http://jabber.org/protocol/disco#items"] = "XEP-0030: Service Discovery (items)";
		Caps2String_ ["http://jabber.org/protocol/ibb"] = "XEP-0047: In-Band Bytestreams";
		Caps2String_ ["jabber:iq:rpc"] = "XEP-0009: Jabber-RPC";
		Caps2String_ ["urn:xmpp:ping"] = "XEP-0199: XMPP Ping";
		Caps2String_ ["jabber:x:conference"] = "XEP-0249: Direct MUC Invitations";
		Caps2String_ ["urn:xmpp:delay"] = "XEP-0203: Delayed Delivery";
		Caps2String_ ["jabber:x:delay"] = "XEP-0091: Legacy Delayed Delivery";
		Caps2String_ ["http://jabber.org/protocol/muc"] = "XEP-0045: Multi-User Chat";
		Caps2String_ ["http://jabber.org/protocol/muc#admin"] = "XEP-0045: Multi-User Chat (Admin)";
		Caps2String_ ["http://jabber.org/protocol/muc#owner"] = "XEP-0045: Multi-User Chat (Owner)";
		Caps2String_ ["http://jabber.org/protocol/muc#user"] = "XEP-0045: Multi-User Chat (User)";
		Caps2String_ ["http://jabber.org/protocol/chatstates"] = "XEP-0085: Chat State Notifications";
		Caps2String_ ["http://jabber.org/protocol/si"] = "XEP-0096: SI File Transfer";
		Caps2String_ ["http://jabber.org/protocol/si/profile/file-transfer"] = "XEP-0096: SI File Transfer (File Transfer profile)";
		Caps2String_ ["http://jabber.org/protocol/feature-neg"] = "XEP-0020: Feature Negotiation";
		Caps2String_ ["http://jabber.org/protocol/bytestreams"] = "XEP-0065: SOCKS5 Bytestreams";
	}
	
	void CapsManager::FetchCaps (const QString& jid, const QByteArray& verNode)
	{
		if (!DB_->Contains (verNode) &&
				verNode.size () > 17)	// 17 is some random number a bit less than 15
			Connection_->RequestInfo (jid);
	}
	
	QStringList CapsManager::GetRawCaps (const QByteArray& verNode) const
	{
		return DB_->Get (verNode);
	}
	
	QStringList CapsManager::GetCaps (const QByteArray& verNode) const
	{
		return GetCaps (GetRawCaps (verNode));
	}
	
	QStringList CapsManager::GetCaps (const QStringList& features) const
	{
		QStringList result;
		Q_FOREACH (const QString& raw, features)
			result << Caps2String_.value (raw, raw);
		result.removeAll (QString ());
		return result;
	}

	void CapsManager::handleInfoReceived (const QXmppDiscoveryIq& iq)
	{
		if (!iq.features ().isEmpty ())
			DB_->Set (iq.verificationString (), iq.features ());
	}
	
	void CapsManager::handleItemsReceived (const QXmppDiscoveryIq& iq)
	{
		if (!iq.features ().isEmpty ())
			DB_->Set (iq.verificationString (), iq.features ());
	}
}
}
}
