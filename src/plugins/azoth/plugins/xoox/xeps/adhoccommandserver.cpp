/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "adhoccommandserver.h"
#include <QDir>
#include <QUrl>
#include <QXmppDiscoveryManager.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include "clientconnection.h"
#include "util.h"
#include "roomclentry.h"
#include "core.h"
#include "glooxmessage.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsCommands = "http://jabber.org/protocol/commands";
	const QString RcStr = "http://jabber.org/protocol/rc";
	const QString NodeChangeStatus = "http://jabber.org/protocol/rc#set-status";
	const QString NodeLeaveGroupchats = "http://jabber.org/protocol/rc#leave-groupchats";
	const QString NodeForward = "http://jabber.org/protocol/rc#forward";

	const QString NodeAddTask = "http://leechcraft.org/plugins-azoth-xoox#add-task";

	AdHocCommandServer::AdHocCommandServer (ClientConnection *conn, IProxyObject *proxy)
	: Conn_ { conn }
	, Proxy_ { proxy }
	{
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

		QXmppDiscoveryIq::Item forward;
		forward.setNode (NodeForward);
		forward.setJid (jid);
		forward.setName (tr ("Forward unread messages"));
		XEP0146Items_ [forward.node ()] = forward;
		NodeInfos_ [forward.node ()] = [this] (QDomElement e) { Forward (e); };

		QXmppDiscoveryIq::Item addTask;
		addTask.setNode (NodeAddTask);
		addTask.setJid (jid);
		addTask.setName (tr ("Add download task"));
		XEP0146Items_ [addTask.node ()] = addTask;
		NodeInfos_ [addTask.node ()] = [this] (QDomElement e) { AddTaskInfo (e); };
		NodeSubmitHandlers_ [addTask.node ()] =
				[this] (QDomElement e, QString s, QXmppDataForm f)
					{ AddTaskSubmitted (e, s, f); };
	}

	bool AdHocCommandServer::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq")
			return false;

		if (elem.attribute ("type") == "set")
			return HandleIqSet (elem);
		else if (QXmppDiscoveryIq::isDiscoveryIq (elem))
			return HandleDiscoIq (elem);

		return false;
	}

	bool AdHocCommandServer::HandleDiscoIq (const QDomElement& elem)
	{
		QXmppDiscoveryIq receivedIq;
        receivedIq.parse (elem);
		if (receivedIq.type () != QXmppIq::Get)
			return false;

		if (receivedIq.queryType () != QXmppDiscoveryIq::ItemsQuery)
			return false;

		if (receivedIq.queryNode () != NsCommands)
			return false;

		auto [from, resource] = ClientConnection::Split (receivedIq.from ());

		if (Conn_->GetOurJID ().startsWith (from))
		{
			QXmppDiscoveryIq result;
			result.setId (receivedIq.id ());
			result.setTo (receivedIq.from ());
			result.setType (QXmppIq::Result);
			result.setQueryNode (NsCommands);
			result.setQueryType (QXmppDiscoveryIq::ItemsQuery);
			result.setItems (XEP0146Items_.values ());
			Conn_->GetClient ()->sendPacket (result);
		}
		else
		{
			QXmppIq error;
			error.setId (receivedIq.id ());
			error.setTo (receivedIq.from ());
			error.setType (QXmppIq::Error);
			error.setError (QXmppStanza::Error (QXmppStanza::Error::Wait, QXmppStanza::Error::Forbidden, "Wrong JID, bro."));
			Conn_->GetClient ()->sendPacket (error);
		}

		return true;
	}

	bool AdHocCommandServer::HandleIqSet (const QDomElement& elem)
	{
		QXmppElement cmdElem = elem.firstChildElement ("command");
		if (cmdElem.attribute ("xmlns") != NsCommands)
			return false;

		if (!cmdElem.attribute ("action").isEmpty () &&
				cmdElem.attribute ("action") != "execute" &&
				cmdElem.attribute ("action") != "complete")
			return false;

		auto [from, resource] = ClientConnection::Split (elem.attribute ("from"));
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
			qWarning () << Q_FUNC_INFO << "no node" << node;
			qWarning () << XEP0146Items_.keys ();
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

	void AdHocCommandServer::Send (const QXmppDataForm& form,
			const QDomElement& sourceElem, const QString& node)
	{
		auto genSessID = [] (const QString& base)
		{
			return base + ":" + QDateTime::currentDateTime ().toString (Qt::ISODate);
		};

		const QString& sessionId = genSessID (sourceElem.attribute ("id"));
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
		iq.setExtensions (QXmppElementList () << elem);

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
		iq.setExtensions (QXmppElementList () << elem);

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

		QList<QPair<State, QString>> rawOpts;
		rawOpts << qMakePair<State, QString> (SChat, "chat");
		rawOpts << qMakePair<State, QString> (SOnline, "online");
		rawOpts << qMakePair<State, QString> (SAway, "away");
		rawOpts << qMakePair<State, QString> (SXA, "xa");
		rawOpts << qMakePair<State, QString> (SDND, "dnd");
		rawOpts << qMakePair<State, QString> (SInvisible, "invisible");
		rawOpts << qMakePair<State, QString> (SOffline, "offline");

		QString option;
		QList<QPair<QString, QString>> options;
		for (const auto& [rawState, status] : rawOpts)
		{
			options << QPair { Proxy_->StateToString (rawState), status };
			if (rawState == state.State_)
				option = status;
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

	namespace
	{
		const auto& GetStr2State ()
		{
			static QMap<QString, State> str2state
			{
				{ "chat", SChat },
				{ "online", SOnline },
				{ "away", SAway },
				{ "xa", SXA },
				{ "dnd", SDND },
				{ "invisible", SInvisible },
				{ "offline", SOffline }
			};
			return str2state;
		}
	}

	void AdHocCommandServer::ChangeStatusSubmitted (const QDomElement& sourceElem,
			const QString& sessionId, const QXmppDataForm& form)
	{
		const GlooxAccountState& origState = Conn_->GetLastState ();
		GlooxAccountState newState = origState;
		for (const auto& field : form.fields ())
		{
			if (field.key () == "status")
				newState.State_ = GetStr2State ().value (field.value ().toString (), newState.State_);
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

		QList<QPair<QString, QString>> options;
		for (auto entryObj : Conn_->GetCLEntries ())
		{
			const auto entry = qobject_cast<RoomCLEntry*> (entryObj);
			if (!entry)
				continue;

			options << QPair
			{
				entry->GetHumanReadableID () + "/" + entry->GetNick (),
				entry->GetEntryID ()
			};
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
		for (const auto& field : form.fields ())
		{
			if (field.key () != "groupchats")
				continue;

			const QStringList& ids = field.value ().toStringList ();
			for (auto entryObj : Conn_->GetCLEntries ())
			{
				auto entry = qobject_cast<RoomCLEntry*> (entryObj);
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

	void AdHocCommandServer::Forward (const QDomElement& sourceElem)
	{
		const QString& to = sourceElem.attribute ("from");

		for (auto obj : Conn_->GetCLEntries ())
		{
			auto base = qobject_cast<EntryBase*> (obj);
			if (!base)
				continue;

			for (auto msgObj : base->GetUnreadMessages ())
			{
				QXmppMessage msg (QString (), to, msgObj->GetNativeMessage ().body ());
				msg.setStamp (msgObj->GetDateTime ());
				msg.setXhtml (msgObj->GetRichBody ());

				const QString& var = msgObj->GetOtherVariant ();
				const QString& from = var.isEmpty () ?
						base->GetHumanReadableID () :
						base->GetHumanReadableID () + '/' + var;

				QXmppExtendedAddress address;
				address.setType ("ofrom");
				address.setJid (from);
				msg.setExtendedAddresses ({ address });

				Conn_->GetClient ()->sendPacket (msg);
			}

			base->MarkMsgsRead ();
		}

		const QString& sess = sourceElem.firstChildElement ("command").attribute ("session");
		SendCompleted (sourceElem, NodeForward, sess);
	}

	void AdHocCommandServer::AddTaskInfo (const QDomElement& sourceElem)
	{
		QList<QXmppDataForm::Field> fields;

		QXmppDataForm::Field field (QXmppDataForm::Field::HiddenField);
		field.setValue (RcStr);
		field.setKey ("FORM_TYPE");
		fields.append (field);

		QXmppDataForm::Field url (QXmppDataForm::Field::TextSingleField);
		url.setLabel ("URL");
		url.setKey ("url");
		url.setRequired (true);
		fields.append (url);

		QXmppDataForm::Field dest (QXmppDataForm::Field::TextSingleField);
		dest.setLabel (tr ("Destination"));
		dest.setKey ("dest");
		dest.setRequired (true);
		dest.setValue (QDir::home ().filePath ("downloads"));
		fields.append (dest);

		QXmppDataForm form (QXmppDataForm::Form);
		form.setTitle (tr ("Add task"));
		form.setInstructions (tr ("Enter task parameters"));
		form.setFields (fields);

		Send (form, sourceElem, NodeAddTask);
	}

	void AdHocCommandServer::AddTaskSubmitted (const QDomElement& sourceElem,
			const QString& sessionId, const QXmppDataForm& form)
	{
		QUrl url;
		QString location;
		for (const auto& field : form.fields ())
		{
			if (field.key () == "url")
				url = QUrl::fromUserInput (field.value ().toString ());
			else if (field.key () == "dest")
				location = field.value ().toString ();
		}

		if (url.isValid () && !location.isEmpty ())
		{
			const auto& e = Util::MakeEntity (url, location, OnlyDownload);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		SendCompleted (sourceElem, NodeAddTask, sessionId);
	}
}
}
}
