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
#include "util.h"
#include "roomclentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsCommands = "http://jabber.org/protocol/commands";
	const QString RcStr = "http://jabber.org/protocol/rc";
	const QString NodeLeaveGroupchats = "http://jabber.org/protocol/rc#leave-groupchats";

	AdHocCommandServer::AdHocCommandServer (ClientConnection *conn)
	: Conn_ (conn)
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
		leaveGroupchats.setNode (NodeLeaveGroupchats);
		leaveGroupchats.setJid (jid);
		leaveGroupchats.setName (tr ("Leave groupchats"));
		XEP0146Items_ [leaveGroupchats.node ()] = leaveGroupchats;
		NodeInfos_ [leaveGroupchats.node ()] =
				boost::bind (&AdHocCommandServer::LeaveGroupchatsInfo, this, _1);
		NodeSubmitHandlers_ [leaveGroupchats.node ()] =
				boost::bind (&AdHocCommandServer::LeaveGroupchatsSubmitted, this, _1, _2, _3);
	}
	
	bool AdHocCommandServer::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq" ||
				elem.attribute ("type") != "set")
			return false;
		
		QXmppElement cmdElem = elem.firstChildElement ("command");
		if (cmdElem.attribute ("xmlns") != NsCommands)
			return false;
		
		if (!cmdElem.attribute ("action").isEmpty () &&
				cmdElem.attribute ("action") != "execute")
			return false;
		
		QString from, resource;
		ClientConnection::Split (elem.attribute ("from"), &from, &resource);
		const bool isUs = Conn_->GetOurJID ().startsWith (from);
		
		const QString& node = cmdElem.attribute ("node");
		
		if (XEP0146Items_.contains (node) && !isUs)
		{
			QXmppIq iq;
			iq.parse (elem);
			iq.setTo (elem.attribute ("from"));
			iq.setFrom (QString ());
			iq.setError (QXmppStanza::Error (QXmppStanza::Error::Auth, QXmppStanza::Error::Forbidden));
			return true;
		}
		
		if (!XEP0146Items_.contains (node))
		{
			QXmppIq iq;
			iq.parse (elem);
			iq.setTo (elem.attribute ("from"));
			iq.setFrom (QString ());
			iq.setError (QXmppStanza::Error (QXmppStanza::Error::Cancel, QXmppStanza::Error::FeatureNotImplemented));
			return true;
		}
		
		const QString& sessionId = cmdElem.attribute ("sessionid");
		if (PendingSessions_ [node].removeAll (sessionId))
		{
			QXmppDataForm form;
			form.parse (XooxUtil::XmppElem2DomElem (cmdElem.firstChildElement ("x")));
			NodeSubmitHandlers_ [node] (elem, sessionId, form);
		}
		else
			NodeInfos_ [node] (elem);
		return true;
	}
	
	void AdHocCommandServer::LeaveGroupchatsInfo (const QDomElement& sourceElem)
	{
		QList<QXmppDataForm::Field> fields;

		QXmppDataForm::Field field (QXmppDataForm::Field::HiddenField);
		field.setValue (RcStr);
		field.setKey ("FORM_TYPE");
		fields.append (field);

		QList<QPair<QString, QString> > options;
		Q_FOREACH (QObject *entryObj, Conn_->GetCLEntries ())
		{
			RoomCLEntry *entry = qobject_cast<RoomCLEntry*> (entryObj);
			if (!entry)
				continue;
			
			QPair<QString, QString> option;
			option.first = entry->GetHumanReadableID () + "/" + entry->GetNick ();
			option.second = entry->GetEntryID ();
			options << option;
		}
		
		QXmppDataForm::Field gcs (QXmppDataForm::Field::ListMultiField);
		gcs.setLabel (tr ("Groupchats"));
		gcs.setKey ("groupchats");
		gcs.setRequired (true);
		gcs.setOptions (options);
		fields.append (gcs);
		
		QXmppDataForm form (QXmppDataForm::Form);
		form.setTitle (tr ("Leave groupchats"));
		form.setInstructions (tr ("Select the groupchats to leave"));
		form.setFields (fields);
		
		const QString& sessionId = "leavegc:" + QDateTime::currentDateTime ().toString (Qt::ISODate);
		PendingSessions_ [NodeLeaveGroupchats] << sessionId;
		
		QXmppElement elem;
		elem.setTagName ("command");
		elem.setAttribute ("xmlns", NsCommands);
		elem.setAttribute ("node", NodeLeaveGroupchats);
		elem.setAttribute ("status", "executing");
		elem.setAttribute ("sessionid", sessionId);
		elem.appendChild (XooxUtil::Form2XmppElem (form));
		
		QXmppIq iq;
		iq.setTo (sourceElem.attribute ("from"));
		iq.setId (sourceElem.attribute ("id"));
		iq.setType (QXmppIq::Result);
		iq.setExtensions (elem);
		
		Conn_->GetClient ()->sendPacket (iq);
	}
	
	void AdHocCommandServer::LeaveGroupchatsSubmitted (const QDomElement& sourceElem,
			const QString& sessionId, const QXmppDataForm& form)
	{
		Q_FOREACH (const QXmppDataForm::Field& field, form.fields ())
		{
			if (field.key () != "groupchats")
				continue;
			
			const QStringList& ids = field.value ().toStringList ();			
			Q_FOREACH (QObject *entryObj, Conn_->GetCLEntries ())
			{
				RoomCLEntry *entry = qobject_cast<RoomCLEntry*> (entryObj);
				if (!entry)
					continue;
				
				if (!ids.contains (entry->GetEntryID ()))
					continue;
				
				entry->Leave (tr ("leaving as the result of the remote command"));
			}

			break;
		}

		QXmppElement elem;
		elem.setTagName ("command");
		elem.setAttribute ("xmlns", NsCommands);
		elem.setAttribute ("node", NodeLeaveGroupchats);
		elem.setAttribute ("status", "completed");
		elem.setAttribute ("sessionid", sessionId);
		
		QXmppIq iq;
		iq.setTo (sourceElem.attribute ("from"));
		iq.setId (sourceElem.attribute ("id"));
		iq.setType (QXmppIq::Result);
		iq.setExtensions (elem);
		
		Conn_->GetClient ()->sendPacket (iq);
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
