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

#include "adhoccommandserver.h"
#include <boost/bind.hpp>
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsCommands = "http://jabber.org/protocol/commands";

	AdHocCommandServer::AdHocCommandServer (ClientConnection *conn)
	: QObject (conn)
	, Conn_ (conn)
	{
		QXmppDiscoveryManager *mgr = conn->GetDiscoveryManager ();
		connect (mgr,
				SIGNAL (itemsReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleDiscoItems (const QXmppDiscoveryIq&)));
		connect (mgr,
				SIGNAL (infoReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleDiscoInfo (const QXmppDiscoveryIq&)));
		
		const QString& jid = Conn_->GetOurJID ();
		
		QXmppDiscoveryIq::Item leaveGroupchats;
		leaveGroupchats.setNode ("http://jabber.org/protocol/rc#leave-groupchats");
		leaveGroupchats.setJid (jid);
		leaveGroupchats.setName (tr ("Leave groupchats"));
		XEP0146Items_ [leaveGroupchats.node ()] = leaveGroupchats;
		NodeInfos_ [leaveGroupchats.node ()] = boost::bind (&AdHocCommandServer::LeaveGroupchatsInfo, this, _1);
	}
	
	void AdHocCommandServer::LeaveGroupchatsInfo (const QXmppDiscoveryIq& iq)
	{
	}

	void AdHocCommandServer::handleDiscoItems (const QXmppDiscoveryIq& iq)
	{
		if (iq.type () != QXmppIq::Get ||
				iq.queryNode () != NsCommands)
			return;
		
		QString from;
		QString resource;
		ClientConnection::Split (iq.from (), &from, &resource);
		
		QList<QXmppDiscoveryIq::Item> items;
		if (Conn_->GetOurJID ().startsWith (from))
			items << XEP0146Items_.values ();
		
		QXmppDiscoveryIq result;
		result.setId (iq.id ());
		result.setTo (iq.from ());
		result.setType (QXmppIq::Result);
		result.setQueryNode (NsCommands);
		result.setQueryType (QXmppDiscoveryIq::ItemsQuery);
		result.setItems (items);
		
		Conn_->GetClient ()->sendPacket (result);
	}

	void AdHocCommandServer::handleDiscoInfo (const QXmppDiscoveryIq& iq)
	{
		if (iq.type () != QXmppIq::Get)
			return;
		
		QString from;
		QString resource;
		ClientConnection::Split (iq.from (), &from, &resource);
	}
}
}
}
