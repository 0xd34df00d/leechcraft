/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "clientconnection.h"
#include <QTimer>
#include <QtDebug>
#include <QXmppClient.h>
#include <QXmppMucManager.h>
#include <QXmppVersionManager.h>
#include <QXmppRosterManager.h>
#include <QXmppVCardManager.h>
#include <QXmppDiscoveryManager.h>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "glooxaccount.h"
#include "config.h"
#include "glooxclentry.h"
#include "glooxmessage.h"
#include "roomhandler.h"
#include "glooxprotocol.h"
#include "core.h"
#include "roomclentry.h"
#include "unauthclentry.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	ClientConnection::ClientConnection (const QString& jid,
			const GlooxAccountState& state,
			GlooxAccount *account)
	: Account_ (account)
	, ProxyObject_ (0)
	, IsConnected_ (false)
	, FirstTimeConnect_ (true)
	, OurJID_ (jid)
	, Client_ (new QXmppClient (this))
	, MUCManager_ (new QXmppMucManager)
	, DiscoveryManager_ (0)
	{
		LastState_.State_ == SOffline;

		QObject *proxyObj = qobject_cast<GlooxProtocol*> (account->
					GetParentProtocol ())->GetProxyObject ();
		ProxyObject_ = qobject_cast<IProxyObject*> (proxyObj);

		Client_->addExtension (MUCManager_);

		Client_->versionManager ().setClientName ("LeechCraft Azoth");
		Client_->versionManager ().setClientVersion (LEECHCRAFT_VERSION);
		Client_->versionManager ().setClientOs (ProxyObject_->GetOSName ());

		connect (Client_,
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));
		connect (Client_,
				SIGNAL (presenceReceived (const QXmppPresence&)),
				this,
				SLOT (handlePresenceChanged (const QXmppPresence&)));
		connect (Client_,
				SIGNAL (messageReceived (const QXmppMessage&)),
				this,
				SLOT (handleMessageReceived (const QXmppMessage&)));

		connect (&Client_->rosterManager (),
				SIGNAL (rosterReceived ()),
				this,
				SLOT (handleRosterReceived ()));
		connect (&Client_->rosterManager (),
				SIGNAL (rosterChanged (const QString&)),
				this,
				SLOT (handleRosterChanged (const QString&)));

		connect (&Client_->vCardManager (),
				SIGNAL (vCardReceived (const QXmppVCardIq&)),
				this,
				SLOT (handleVCardReceived (const QXmppVCardIq&)));

		DiscoveryManager_ = Client_->findExtension<QXmppDiscoveryManager> ();
		if (!DiscoveryManager_)
		{
			DiscoveryManager_ = new QXmppDiscoveryManager ();
			Client_->addExtension (DiscoveryManager_);
		}
		DiscoveryManager_->setClientCapabilitiesNode ("http://leechcraft.org/azoth");
		connect (DiscoveryManager_,
				SIGNAL (infoReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleInfoReceived (const QXmppDiscoveryIq&)));

		connect (MUCManager_,
				SIGNAL (roomPermissionsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&)),
				this,
				SLOT (handleRoomPermissionsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&)));
	}

	ClientConnection::~ClientConnection ()
	{
		qDeleteAll (RoomHandlers_);
	}

	void ClientConnection::SetState (const GlooxAccountState& state)
	{
		LastState_ = state;

		QXmppPresence::Type presType = state.State_ == SOffline ?
				QXmppPresence::Unavailable :
				QXmppPresence::Available;
		QXmppPresence pres (presType,
				QXmppPresence::Status (static_cast<QXmppPresence::Status::Type> (state.State_),
						state.Status_,
						state.Priority_));
		Client_->setClientPresence (pres);
		Q_FOREACH (RoomHandler *rh, RoomHandlers_)
			rh->SetState (state);

		if (!IsConnected_ &&
				state.State_ != SOffline)
		{
			if (FirstTimeConnect_)
				emit needPassword ();

			Client_->connectToServer (OurJID_, Password_);

			FirstTimeConnect_ = false;
		}

		if (state.State_ == SOffline)
		{
			IsConnected_ = false;
			Q_FOREACH (const QString& jid, JID2CLEntry_.keys ())
			{
				GlooxCLEntry *entry = JID2CLEntry_.take (jid);
				ODSEntries_ [jid] = entry;
				entry->Convert2ODS ();
			}
		}
	}

	void ClientConnection::Synchronize ()
	{
	}

	void ClientConnection::SetPassword (const QString& pwd)
	{
		Password_ = pwd;
	}

	QString ClientConnection::GetOurJID () const
	{
		return OurJID_;
	}

	RoomCLEntry* ClientConnection::JoinRoom (const QString& jid, const QString& nick)
	{
		if (RoomHandlers_.contains (jid))
		{
			Entity e = Util::MakeNotification ("Azoth",
					tr ("This room is already joined."),
					PCritical_);
			Core::Instance ().SendEntity (e);
			return 0;
		}

		RoomHandler *rh = new RoomHandler (jid, nick, Account_);
		MUCManager_->joinRoom (jid, nick);
		//rh->SetState (LastState_);

		RoomHandlers_ [jid] = rh;

		return rh->GetCLEntry ();
	}

	void ClientConnection::Unregister (RoomHandler *rh)
	{
		RoomHandlers_.remove (rh->GetRoomJID ());
	}

	QXmppMucManager* ClientConnection::GetMUCManager () const
	{
		return MUCManager_;
	}

	void ClientConnection::RequestInfo (const QString& jid) const
	{
		qDebug () << "requesting info for" << jid;
		DiscoveryManager_->requestInfo (jid);
	}

	void ClientConnection::Update (const QXmppRosterIq::Item& item)
	{
		QXmppRosterIq iq;
		iq.setType (QXmppIq::Set);
		iq.addItem (item);
		Client_->sendPacket (iq);
	}

	void ClientConnection::Update (const QXmppMucAdminIq::Item& item)
	{
		QXmppMucAdminIq iq;
		iq.setType (QXmppIq::Set);
		iq.setItems (QList<QXmppMucAdminIq::Item> () << item);
		Client_->sendPacket (iq);
	}

	void ClientConnection::AckAuth (QObject *entryObj, bool ack)
	{
		UnauthCLEntry *entry = qobject_cast<UnauthCLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "is not an UnauthCLEntry";
			return;
		}

		QXmppPresence pres;
		pres.setType (ack ? QXmppPresence::Subscribed : QXmppPresence::Unsubscribed);
		pres.setTo (entry->GetJID ());
		Client_->sendPacket (pres);

		emit rosterItemRemoved (entry);
		entry->deleteLater ();
	}

	void ClientConnection::Subscribe (const QString& id,
			const QString& msg, const QString& name, const QStringList& groups)
	{
		QXmppPresence pres;
		pres.setType (QXmppPresence::Subscribe);
		pres.setTo (id);
		Client_->sendPacket (pres);
	}

	void ClientConnection::RevokeSubscription (const QString& jid, const QString& reason)
	{
		QXmppPresence pres;
		pres.setType (QXmppPresence::Unsubscribe);
		pres.setTo (jid);
		Client_->sendPacket (pres);
	}

	void ClientConnection::Unsubscribe (const QString& jid, const QString& reason)
	{
		QXmppPresence presence;
		presence.setType (QXmppPresence::Unsubscribed);
		presence.setTo (jid);
		Client_->sendPacket (presence);
	}

	void ClientConnection::Remove (GlooxCLEntry *entry)
	{
		Client_->rosterManager ().removeRosterEntry (entry->GetJID ());
	}

	QXmppClient* ClientConnection::GetClient () const
	{
		return Client_;
	}

	QObject* ClientConnection::GetCLEntry (const QString& bareJid, const QString& variant) const
	{
		if (RoomHandlers_.contains (bareJid))
			return RoomHandlers_ [bareJid]->GetParticipantEntry (variant).get ();
		else
			return JID2CLEntry_ [bareJid];
	}

	GlooxCLEntry* ClientConnection::AddODSCLEntry (GlooxCLEntry::OfflineDataSource_ptr ods)
	{
		GlooxCLEntry *entry = new GlooxCLEntry (ods, Account_);
		ODSEntries_ [ods->ID_] = entry;

		emit gotRosterItems (QList<QObject*> () << entry);

		return entry;
	}

	QList<QObject*> ClientConnection::GetCLEntries () const
	{
		QList<QObject*> result;
		Q_FOREACH (GlooxCLEntry *entry, JID2CLEntry_.values () + ODSEntries_.values ())
			result << entry;
		Q_FOREACH (RoomHandler *rh, RoomHandlers_)
		{
			result << rh->GetCLEntry ();
			result << rh->GetParticipants ();
		}
		return result;
	}

	void ClientConnection::FetchVCard (const QString& jid)
	{
		Client_->vCardManager ().requestVCard (jid);
	}

	GlooxMessage* ClientConnection::CreateMessage (IMessage::MessageType type,
			const QString& resource, const QString& body, const QXmppRosterIq::Item& item)
	{
		GlooxMessage *msg = new GlooxMessage (type,
				IMessage::DOut,
				item.bareJid (),
				resource,
				this);
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());
		return msg;
	}

	EntryStatus ClientConnection::PresenceToStatus (const QXmppPresence& pres) const
	{
		const QXmppPresence::Status& status = pres.status ();
		EntryStatus st (static_cast<State> (status.type ()), status.statusText ());
		if (pres.type () == QXmppPresence::Unavailable)
			st.State_ = SOffline;
		return st;
	}

	void ClientConnection::Split (const QString& jid,
			QString *bare, QString *resource) const
	{
		const QStringList& splitted = jid.split ('/', QString::SkipEmptyParts);
		*bare = splitted.at (0);
		*resource = splitted.value (1);
	}

	void ClientConnection::handleConnected ()
	{
		IsConnected_ = true;
	}

	void ClientConnection::handleRosterReceived ()
	{
		QXmppRosterManager& rm = Client_->rosterManager ();
		QObjectList items;
		Q_FOREACH (const QString& bareJid,
				rm.getRosterBareJids ())
			items << CreateCLEntry (rm.getRosterEntry (bareJid));
		emit gotRosterItems (items);
	}

	void ClientConnection::handleRosterChanged (const QString& bareJid)
	{
		QXmppRosterManager& rm = Client_->rosterManager ();
		QMap<QString, QXmppPresence> presences = rm.getAllPresencesForBareJid (bareJid);
		GlooxCLEntry *entry = JID2CLEntry_ [bareJid];
		Q_FOREACH (const QString& resource, presences.keys ())
		{
			const QXmppPresence& pres = presences [resource];
			entry->SetClientInfo (resource, pres);
			entry->SetStatus (PresenceToStatus (pres), resource);
		}
		entry->UpdateRI (rm.getRosterEntry (bareJid));
		qDebug () << "roster changed" << bareJid;
		Core::Instance ().saveRoster ();
	}

	void ClientConnection::handleVCardReceived (const QXmppVCardIq& vcard)
	{
		QString jid;
		QString nick;
		Split (vcard.from (), &jid, &nick);
		if (JID2CLEntry_.contains (jid))
			JID2CLEntry_ [jid]->SetVCard (vcard);
		else if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->GetParticipantEntry (nick)->SetVCard (vcard);
		else
			qWarning () << Q_FUNC_INFO
					<< "could not find entry for"
					<< vcard.from ();
	}

	void ClientConnection::handleInfoReceived (const QXmppDiscoveryIq& iq)
	{
		qDebug () << Q_FUNC_INFO << iq.from ();
		qDebug () << iq.features ();
		Q_FOREACH (const QXmppDiscoveryIq::Item& item, iq.items ())
			qDebug () << item.jid () << item.name () << item.node ();
		Q_FOREACH (const QXmppDiscoveryIq::Identity& id, iq.identities ())
			qDebug () << id.name () << id.type () << id.category () << id.language ();
	}

	void ClientConnection::handlePresenceChanged (const QXmppPresence& pres)
	{
		if (pres.type () != QXmppPresence::Unavailable &&
				pres.type () != QXmppPresence::Available)
		{
			HandleOtherPresence (pres);
			return;
		}

		QString jid;
		QString resource;
		Split (pres.from (), &jid, &resource);

		if (!JID2CLEntry_.contains (jid))
		{
			if (ODSEntries_.contains (jid))
				ConvertFromODS (jid, Client_->rosterManager ().getRosterEntry (jid));
			else if (RoomHandlers_.contains (jid))
			{
				RoomHandlers_ [jid]->HandlePresence (pres, resource);
				return;
			}
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "no entry for"
						<< jid
						<< resource;
				return;
			}
		}

		JID2CLEntry_ [jid]->SetClientInfo (resource, pres);
		JID2CLEntry_ [jid]->SetStatus (PresenceToStatus (pres), resource);
	}

	void ClientConnection::handleMessageReceived (const QXmppMessage& msg)
	{
		QString jid;
		QString resource;
		Split (msg.from (), &jid, &resource);

		if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->HandleMessage (msg, resource);
		else if (JID2CLEntry_.contains (jid))
		{
			GlooxMessage *gm = new GlooxMessage (msg, this);
			JID2CLEntry_ [jid]->HandleMessage (gm);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "could not find source for"
					<< msg.from ();
	}

	void ClientConnection::handleRoomPermissionsReceived (const QString& roomJid,
			const QList<QXmppMucAdminIq::Item>& perms)
	{
		if (!RoomHandlers_.contains (roomJid))
		{
			qWarning () << Q_FUNC_INFO
					<< "no RoomHandler for"
					<< roomJid
					<< RoomHandlers_.keys ();
			return;
		}

		RoomHandlers_ [roomJid]->SetState (LastState_);
		RoomHandlers_ [roomJid]->UpdatePerms (perms);
	}

	void ClientConnection::HandleOtherPresence (const QXmppPresence& pres)
	{
		switch (pres.type ())
		{
		case QXmppPresence::Subscribe:
			break;
		}
	}

	/*
	void ClientConnection::onConnect ()
	{
		Q_FOREACH (RoomHandler *rh, RoomHandlers_)
		{
			gloox::JID jid = rh->GetRoomJID ();
			// cache room parameters
			QString server = QString (jid.server().c_str ());
			QString room = QString (jid.username ().c_str ());
			QString nick = rh->GetCLEntry ()->GetNick ();
			// leave conference
			rh->GetCLEntry ()->Leave (QString ());
			// join again
			Account_->JoinRoom (server, room, nick);
		}
		IsConnected_ = true;
	}

	void ClientConnection::handleItemAdded (const gloox::JID& jid)
	{
		qDebug () << Q_FUNC_INFO << jid.full ().c_str ();
		gloox::RosterItem *ri = Client_->rosterManager ()->getRosterItem (jid);

		GlooxCLEntry *entry = CreateCLEntry (ri);
		emit gotRosterItems (QList<QObject*> () << entry);
	}

	void ClientConnection::handleItemSubscribed (const gloox::JID& jid)
	{
		qDebug () << Q_FUNC_INFO << jid.full ().c_str ();
		handleItemAdded (jid);

		emit rosterItemSubscribed (JID2CLEntry_ [jid.bareJID ()]);
	}

	void ClientConnection::handleItemRemoved (const gloox::JID& jid)
	{
		qDebug () << Q_FUNC_INFO << jid.full ().c_str ();
		if (!JID2CLEntry_.contains (jid.bareJID ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "strange, we have no"
					<< jid.full ().c_str ()
					<< "in our JID2CLEntry_";
			return;
		}

		GlooxCLEntry *entry = JID2CLEntry_.take (jid.bareJID ());
		emit rosterItemRemoved (entry);
	}

	void ClientConnection::handleItemUpdated (const gloox::JID& jid)
	{
		qDebug () << Q_FUNC_INFO << jid.full ().c_str ();
		if (!JID2CLEntry_.contains (jid.bareJID ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "strange, we have no"
					<< jid.full ().c_str ()
					<< "in our JID2CLEntry_";
			return;
		}

		FetchVCard (jid);

		emit rosterItemUpdated (JID2CLEntry_ [jid.bareJID ()]);
	}

	bool ClientConnection::handleSubscriptionRequest (const gloox::JID& jid, const std::string& msg)
	{
		const std::string& bare = jid.bare ();
		qDebug () << Q_FUNC_INFO << bare.c_str ();
		const QString& str = QString::fromUtf8 (msg.c_str ());
		emit gotSubscriptionRequest (new UnauthCLEntry (jid, str, Account_),
				str);
		return false;
	}

	void ClientConnection::handleVCard (const gloox::JID& jid, const gloox::VCard *vcard)
	{
		if (!vcard)
		{
			qWarning () << Q_FUNC_INFO
					<< "got null vcard"
					<< "for jid"
					<< jid.full ().c_str ();
			return;
		}

		Q_FOREACH (RoomHandler *rh, RoomHandlers_)
			if (rh->GetRoomJID () == jid.bare ())
			{
				rh->HandleVCard (vcard,
					QString::fromUtf8 (jid.resource ().c_str ()));
				return;
			}

		if (JID2CLEntry_.contains (jid))
		{
			JID2CLEntry_ [jid]->SetVCard (vcard);
			JID2CLEntry_ [jid]->SetAvatar (vcard->photo ());
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "vcard reply for unknown request for jid"
					<< jid.full ().c_str ();
	}
	*/

	GlooxCLEntry* ClientConnection::CreateCLEntry (const QXmppRosterIq::Item& ri)
	{
		GlooxCLEntry *entry = 0;
		const QString& bareJID = ri.bareJid ();
		if (!JID2CLEntry_.contains (bareJID))
		{
			if (ODSEntries_.contains (bareJID))
				entry = ConvertFromODS (bareJID, ri);
			else
			{
				entry = new GlooxCLEntry (bareJID, Account_);
				JID2CLEntry_ [bareJID] = entry;
				FetchVCard (bareJID);
			}
		}
		else
		{
			entry = JID2CLEntry_ [bareJID];
			entry->UpdateRI (ri);
		}
		return entry;
	}

	GlooxCLEntry* ClientConnection::ConvertFromODS (const QString& bareJID,
			const QXmppRosterIq::Item& ri)
	{
		GlooxCLEntry *entry = ODSEntries_.take (bareJID);
		entry->UpdateRI (ri);
		JID2CLEntry_ [bareJID] = entry;
		if (entry->GetAvatar ().isNull ())
			FetchVCard (bareJID);
		return entry;
	}
}
}
}
}
}
