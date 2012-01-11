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

#include "adhoccommandmanager.h"
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"
#include "util.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsAdHoc = "http://jabber.org/protocol/commands";

	QString AdHocCommandManager::GetAdHocFeature ()
	{
		return NsAdHoc;
	}

	AdHocCommandManager::AdHocCommandManager (ClientConnection *conn)
	: ClientConn_ (conn)
	{
		connect (ClientConn_->GetDiscoveryManager (),
				SIGNAL (itemsReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleItemsReceived (const QXmppDiscoveryIq&)));
	}

	void AdHocCommandManager::QueryCommands (const QString& jid)
	{
		ClientConn_->GetDiscoveryManager ()->requestItems (jid, NsAdHoc);
	}

	void AdHocCommandManager::ExecuteCommand (const QString& jid,
			const AdHocCommand& cmd)
	{
		QXmppElement command;
		command.setTagName ("command");
		command.setAttribute ("xmlns", NsAdHoc);
		command.setAttribute ("node", cmd.GetNode ());
		command.setAttribute ("action", "execute");

		QXmppIq iq (QXmppIq::Set);
		iq.setTo (jid);
		iq.setExtensions (command);

		PendingCommands_ << iq.id ();
		client ()->sendPacket (iq);
	}

	void AdHocCommandManager::ProceedExecuting (const QString& jid,
			const AdHocResult& state, const QString& action)
	{
		QXmppElement command;
		command.setTagName ("command");
		command.setAttribute ("xmlns", NsAdHoc);
		command.setAttribute ("node", state.GetNode ());
		command.setAttribute ("sessionid", state.GetSessionID ());
		command.setAttribute ("action", action);

		QDomDocument formElem;
		{
			QByteArray ba;
			QXmlStreamWriter w (&ba);
			QXmppDataForm form = state.GetDataForm ();
			form.setType (QXmppDataForm::Submit);
			form.toXml (&w);
			formElem.setContent (ba);
		}
		command.appendChild (formElem.documentElement ());

		QXmppIq iq (QXmppIq::Set);
		iq.setTo (jid);
		iq.setExtensions (command);

		PendingCommands_ << iq.id ();
		client ()->sendPacket (iq);
	}

	QStringList AdHocCommandManager::discoveryFeatures () const
	{
		return QStringList (NsAdHoc);
	}

	bool AdHocCommandManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq" ||
				!PendingCommands_.contains (elem.attribute ("id")))
			return false;

		PendingCommands_.remove (elem.attribute ("id"));

		const QDomElement& command = elem.firstChildElement ("command");
		if (command.namespaceURI () != NsAdHoc)
			return false;

		AdHocResult result;
		result.SetSessionID (command.attribute ("sessionid"));
		result.SetNode (command.attribute ("node"));

		const QDomElement& actionsElem = command.firstChildElement ("actions");
		if (!actionsElem.isNull ())
		{
			QStringList actionsList;

			QString def = actionsElem.attribute ("execute");
			if (def.isEmpty ())
				def = "execute";

			QDomElement actionElem = actionsElem.firstChildElement ();
			while (!actionElem.isNull ())
			{
				const QString& name = actionElem.tagName ();
				if (name != def)
					actionsList << name;

				actionElem = actionElem.nextSiblingElement ();
			}

			actionsList.prepend (def);

			result.SetActions (actionsList);
		}
		else if (command.attribute ("status") == "executing")
			result.SetActions (QStringList ("execute"));

		if (command.firstChildElement ("x").namespaceURI () == "jabber:x:data")
		{
			QXmppDataForm form;

			QDomElement xForm = command.firstChildElement ("x");
			if (!xForm.hasAttribute ("type"))
			{
				qWarning () << Q_FUNC_INFO
						<< "kludge for unknown form types";
				xForm.setAttribute ("type", "form");
			}

			form.parse (xForm);
			result.SetDataForm (form);
		}

		emit gotResult (elem.attribute ("from"), result);

		return true;
	}

	void AdHocCommandManager::handleItemsReceived (const QXmppDiscoveryIq& iq)
	{
		if (iq.queryNode () != NsAdHoc)
			return;

		QList<AdHocCommand> commands;
		Q_FOREACH (const QXmppDiscoveryIq::Item& item, iq.items ())
		{
			AdHocCommand cmd (item.name (), item.node ());
			commands << cmd;
		}

		emit gotCommands (iq.from (), commands);
	}
}
}
}
