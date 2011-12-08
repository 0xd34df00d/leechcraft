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
#include <QXmppDiscoveryManager.h>
#include "clientconnection.h"
#include "util.h"
#include "roomclentry.h"
#include "core.h"
#include <interfaces/iproxyobject.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsCommands = "http://jabber.org/protocol/commands";
	const QString RcStr = "http://jabber.org/protocol/rc";
	const QString NodeChangeStatus = "ttp://jabber.org/protocol/rc#set-status";
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

		QXmppDiscoveryIq::Item changeStatus;
		changeStatus.setNode (NodeChangeStatus);
		changeStatus.setJid (jid);
		changeStatus.setName (tr ("Change status"));
		XEP0146Items_ [changeStatus.node ()] = changeStatus;
		NodeInfos_ [changeStatus.node ()] = [this] (QDomElement e) { ChangeStatusInfo (e); };
		NodeSubmitHandlers_ [changeStatus.node ()] =
				[this] (QDomElement e, QString s, QXmppDataForm f)
					{ ChangeStatusSubmitted (e, s, f); };

		QXmppDiscoveryIq::Item leaveGroupchats;
		leaveGroupchats.setNode (NodeLeaveGroupchats);
		leaveGroupchats.setJid (jid);
		leaveGroupchats.setName (tr ("Leave groupchats"));
		XEP0146Items_ [leaveGroupchats.node ()] = leaveGroupchats;
		NodeInfos_ [leaveGroupchats.node ()] = [this] (QDomElement e) { LeaveGroupchatsInfo (e); };
		NodeSubmitHandlers_ [leaveGroupchats.node ()] =
				[this] (QDomElement e, QString s, QXmppDataForm f)
					{ LeaveGroupchatsSubmitted (e, s, f); };
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

	namespace
	{
		QString GenSessID (const QString& base)
		{
			return base + ":" + QDateTime::currentDateTime ().toString (Qt::ISODate);
		}
	}

	void AdHocCommandServer::Send (const QXmppDataForm& form,
			const QDomElement& sourceElem, const QString& node)
	{
		const QString& sessionId = GenSessID (sourceElem.attribute ("id"));
		PendingSessions_ [node] << sessionId;

		QXmppElement elem;
		elem.setTagName ("command");
		elem.setAttribute ("xmlns", NsCommands);
		elem.setAttribute ("node", node);
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

	void AdHocCommandServer::SendCompleted (const QDomElement& sourceElem,
			const QString& node, const QString& sessionId)
	{
		QXmppElement elem;
		elem.setTagName ("command");
		elem.setAttribute ("xmlns", NsCommands);
		elem.setAttribute ("node", node);
		elem.setAttribute ("status", "completed");
		elem.setAttribute ("sessionid", sessionId);

		QXmppIq iq;
		iq.setTo (sourceElem.attribute ("from"));
		iq.setId (sourceElem.attribute ("id"));
		iq.setType (QXmppIq::Result);
		iq.setExtensions (elem);

		Conn_->GetClient ()->sendPacket (iq);
	}

	void AdHocCommandServer::ChangeStatusInfo (const QDomElement& sourceElem)
	{
		QList<QXmppDataForm::Field> fields;

		QXmppDataForm::Field field (QXmppDataForm::Field::HiddenField);
		field.setValue (RcStr);
		field.setKey ("FORM_TYPE");
		fields << field;

		const GlooxAccountState& state = Conn_->GetLastState ();

		QList<QPair<State, QString> > rawOpts;
		rawOpts << qMakePair<State, QString> (SChat, "chat");
		rawOpts << qMakePair<State, QString> (SOnline, "online");
		rawOpts << qMakePair<State, QString> (SAway, "away");
		rawOpts << qMakePair<State, QString> (SXA, "xa");
		rawOpts << qMakePair<State, QString> (SDND, "dnd");
		rawOpts << qMakePair<State, QString> (SInvisible, "invisible");
		rawOpts << qMakePair<State, QString> (SOffline, "offline");

		QString option;
		QList<QPair<QString, QString> > options;
		QPair<State, QString> pair;
		IProxyObject *proxy = Core::Instance ().GetPluginProxy ();
		Q_FOREACH (pair, rawOpts)
		{
			options << qMakePair (proxy->StateToString (pair.first), pair.second);
			if (pair.first == state.State_)
				option = pair.second;
		}

		QXmppDataForm::Field stateField (QXmppDataForm::Field::ListSingleField);
		stateField.setLabel (tr ("Status"));
		stateField.setRequired (true);
		stateField.setKey ("status");
		stateField.setOptions (options);
		stateField.setValue (option);
		fields << stateField;

		QXmppDataForm::Field prioField (QXmppDataForm::Field::TextSingleField);
		prioField.setLabel (tr ("Priority"));
		prioField.setKey ("status-priority");
		prioField.setValue (QString::number (state.Priority_));
		fields << prioField;

		QXmppDataForm::Field msgField (QXmppDataForm::Field::TextMultiField);
		msgField.setLabel (tr ("Status message"));
		msgField.setKey ("status-message");
		msgField.setValue (state.Status_);
		fields << msgField;

		QXmppDataForm form (QXmppDataForm::Form);
		form.setTitle (tr ("Change status"));
		form.setInstructions (tr ("Choose the new status, priority and status message"));
		form.setFields (fields);

		Send (form, sourceElem, NodeChangeStatus);
	}

	void AdHocCommandServer::ChangeStatusSubmitted (const QDomElement& sourceElem,
			const QString& sessionId, const QXmppDataForm& form)
	{
		const GlooxAccountState& origState = Conn_->GetLastState ();
		GlooxAccountState newState = origState;
		Q_FOREACH (const QXmppDataForm::Field& field, form.fields ())
		{
			if (field.key () == "status")
			{
				QMap<QString, State> str2state;
				str2state ["chat"] = SChat;
				str2state ["online"] = SOnline;
				str2state ["away"] = SAway;
				str2state ["xa"] = SXA;
				str2state ["dnd"] = SDND;
				str2state ["invisible"] = SInvisible;
				str2state ["offline"] = SOffline;

				newState.State_ = str2state.value (field.value ().toString (), newState.State_);
			}
			else if (field.key () == "status-priority")
				newState.Priority_ = field.value ().toInt ();
			else if (field.key () == "status-message")
				newState.Status_ = field.value ().toString ();
		}

		if (newState == Conn_->GetLastState ())
			return;

		Conn_->SetState (newState);

		SendCompleted (sourceElem, NodeChangeStatus, sessionId);
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

		Send (form, sourceElem, NodeLeaveGroupchats);
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

		SendCompleted (sourceElem, NodeLeaveGroupchats, sessionId);
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
