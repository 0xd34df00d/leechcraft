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
#include <QXmppTransferManager.h>
#include <QXmppReconnectionManager.h>
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
namespace Azoth
{
namespace Xoox
{
	const int ErrorLimit = 5;
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
	, XferManager_ (new QXmppTransferManager)
	, DiscoveryManager_ (0)
	, SocketErrorAccumulator_ (0)
	{
		LastState_.State_ = SOffline;
		
		QTimer *decrTimer = new QTimer (this);
		connect (decrTimer,
				SIGNAL (timeout ()),
				this,
				SLOT (decrementErrAccumulators ()));
		decrTimer->start (15000);

		QObject *proxyObj = qobject_cast<GlooxProtocol*> (account->
					GetParentProtocol ())->GetProxyObject ();
		ProxyObject_ = qobject_cast<IProxyObject*> (proxyObj);

		Client_->addExtension (MUCManager_);
		Client_->addExtension (XferManager_);

		DiscoveryManager_ = Client_->findExtension<QXmppDiscoveryManager> ();
		DiscoveryManager_->setClientCapabilitiesNode ("http://leechcraft.org/azoth");

		Client_->versionManager ().setClientName ("LeechCraft Azoth");
		Client_->versionManager ().setClientVersion (LEECHCRAFT_VERSION);
		Client_->versionManager ().setClientOs (ProxyObject_->GetOSName ());

		connect (Client_,
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));
		connect (Client_,
				SIGNAL (error (QXmppClient::Error)),
				this,
				SLOT (handleError (QXmppClient::Error)));
		connect (Client_,
				SIGNAL (iqReceived (const QXmppIq&)),
				this,
				SLOT (handleIqReceived (const QXmppIq&)));
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
		connect (&Client_->rosterManager (),
				SIGNAL (itemRemoved (const QString&)),
				this,
				SLOT (handleRosterItemRemoved (const QString&)));

		connect (&Client_->vCardManager (),
				SIGNAL (vCardReceived (const QXmppVCardIq&)),
				this,
				SLOT (handleVCardReceived (const QXmppVCardIq&)));

		connect (DiscoveryManager_,
				SIGNAL (infoReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleInfoReceived (const QXmppDiscoveryIq&)));

		connect (MUCManager_,
				SIGNAL (roomPermissionsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&)),
				this,
				SLOT (handleRoomPermissionsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&)));
		connect (MUCManager_,
				SIGNAL (roomParticipantNickChanged (const QString&, const QString&, const QString&)),
				this,
				SLOT (handleRoomPartNickChange (const QString&, const QString&, const QString&)));
		connect (MUCManager_,
				SIGNAL (roomPresenceChanged (const QString&, const QString&, const QXmppPresence&)),
				this,
				SLOT (handleRoomPresenceChanged (const QString&, const QString&, const QXmppPresence&)));
		connect (MUCManager_,
				SIGNAL (roomParticipantPermsChanged (const QString&, const QString&,
						QXmppMucAdminIq::Item::Affiliation,
						QXmppMucAdminIq::Item::Role,
						const QString&)),
				this,
				SLOT (handleRoomParticipantPermsChanged (const QString&, const QString&,
						QXmppMucAdminIq::Item::Affiliation,
						QXmppMucAdminIq::Item::Role,
						const QString&)));
		
		connect (Client_->reconnectionManager (),
				SIGNAL (reconnectingIn (int)),
				this,
				SLOT (handleReconnecting (int)));
		connect (Client_->reconnectionManager (),
				SIGNAL (reconnectingNow ()),
				this,
				SLOT (handleReconnecting ()));
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

			QXmppConfiguration conf;
			conf.setJid (OurJID_);
			conf.setPassword (Password_);
			const QString& host = Account_->GetHost ();
			const int port = Account_->GetPort ();
			if (!host.isEmpty ())
				conf.setHost (host);
			if (port >= 0)
				conf.setPort (port);
			Client_->connectToServer (conf, pres);

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

	void ClientConnection::SetOurJID (const QString& jid)
	{
		OurJID_ = jid;
	}

	/** @todo Set the correct state on join.
	 *
	 * Requires proper support for this from the QXmpp part.
	 */
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

	QXmppTransferManager* ClientConnection::GetTransferManager () const
	{
		return XferManager_;
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

	void ClientConnection::Update (const QXmppMucAdminIq::Item& item, const QString& room)
	{
		QXmppMucAdminIq iq;
		iq.setTo (room);
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

		const QString& jid = entry->GetJID ();

		qDebug () << "AckAuth" << jid << ack;

		if (ack)
		{
			Client_->rosterManager ().grantSubscription (jid);
			Subscribe (jid, QString (), entry->GetEntryName (), entry->Groups ());
		}
		else
			Client_->rosterManager ().cancelSubscription (jid);

		emit rosterItemRemoved (entry);
		entry->deleteLater ();
	}

	void ClientConnection::Subscribe (const QString& id,
			const QString& msg, const QString& name, const QStringList& groups)
	{
		qDebug () << "Subscribe" << id;
		if (!Client_->rosterManager ().getRosterBareJids ().contains (id))
			Client_->rosterManager ().addRosterEntry (id,
					name, msg, QSet<QString>::fromList (groups));
		Client_->rosterManager ().subscribe (id, msg);
	}

	void ClientConnection::RevokeSubscription (const QString& jid, const QString& reason)
	{
		qDebug () << "RevokeSubscription" << jid;
		Client_->rosterManager ().cancelSubscription (jid, reason);
	}

	void ClientConnection::Unsubscribe (const QString& jid, const QString& reason)
	{
		qDebug () << "Unsubscribe" << jid;
		Client_->rosterManager ().unsubscribe (jid, reason);
	}

	void ClientConnection::Remove (GlooxCLEntry *entry)
	{
		const QString& jid = entry->GetJID ();

		Client_->rosterManager ().removeRosterEntry (jid);

		if (ODSEntries_.contains (jid))
			delete ODSEntries_.take (jid);
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
			QString *bare, QString *resource)
	{
		const int pos = jid.indexOf ('/');
		*bare = jid.left (pos);
		*resource = (pos >= 0 ? jid.mid (pos + 1) : QString ());
	}

	void ClientConnection::handleConnected ()
	{
		IsConnected_ = true;
	}
	
	void ClientConnection::handleReconnecting (int timeout)
	{
		qDebug () << "Azoth: reconnecting in"
				<< (timeout >= 0 ? QString::number (timeout).toLatin1 () : "now");
	}

	void ClientConnection::handleError (QXmppClient::Error error)
	{
		QString str;
		switch (error)
		{
		case QXmppClient::SocketError:
			if (SocketErrorAccumulator_ < ErrorLimit)
			{
				++SocketErrorAccumulator_;
				str = tr ("Socket error %1.")
						.arg (Client_->socketError ());
			}
			break;
		case QXmppClient::KeepAliveError:
			str = tr ("Keep-alive error.");
			break;
		case QXmppClient::XmppStreamError:
			str = tr ("Error while connecting: ");
			str += HandleErrorCondition (Client_->xmppStreamError ());
			break;
		}
		
		if (str.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "suppressed"
					<< str
					<< error
					<< Client_->socketError ()
					<< Client_->xmppStreamError ();
			return;
		}

		const Entity& e = Util::MakeNotification ("Azoth",
				str,
				PCritical_);
		Core::Instance ().SendEntity (e);
	}

	void ClientConnection::handleIqReceived (const QXmppIq& iq)
	{
		if (iq.error ().isValid ())
			HandleError (iq);
	}

	void ClientConnection::handleRosterReceived ()
	{
		QXmppRosterManager& rm = Client_->rosterManager ();
		QObjectList items;
		Q_FOREACH (const QString& bareJid,
				rm.getRosterBareJids ())
		{
			QXmppRosterIq::Item re = rm.getRosterEntry (bareJid);
			GlooxCLEntry *entry = CreateCLEntry (re);
			items << entry;
			QMap<QString, QXmppPresence> presences = rm.getAllPresencesForBareJid (re.bareJid ());
			Q_FOREACH (const QString& resource, presences.keys ())
				entry->SetClientInfo (resource, presences [resource]);
		}
		emit gotRosterItems (items);
	}

	void ClientConnection::handleRosterChanged (const QString& bareJid)
	{
		QXmppRosterManager& rm = Client_->rosterManager ();
		QMap<QString, QXmppPresence> presences = rm.getAllPresencesForBareJid (bareJid);

		if (!JID2CLEntry_.contains (bareJid))
			emit gotRosterItems (QObjectList () << CreateCLEntry (bareJid));

		GlooxCLEntry *entry = JID2CLEntry_ [bareJid];
		Q_FOREACH (const QString& resource, presences.keys ())
		{
			const QXmppPresence& pres = presences [resource];
			entry->SetClientInfo (resource, pres);
			entry->SetStatus (PresenceToStatus (pres), resource);
		}
		entry->UpdateRI (rm.getRosterEntry (bareJid));
		Core::Instance ().saveRoster ();
	}

	void ClientConnection::handleRosterItemRemoved (const QString& bareJid)
	{
		qDebug () << "RosterItemRemoved" << bareJid;
		if (!JID2CLEntry_.contains (bareJid))
			return;

		GlooxCLEntry *entry = JID2CLEntry_.take (bareJid);
		emit rosterItemRemoved (entry);
		entry->deleteLater ();

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
			else
				return;
		}

		JID2CLEntry_ [jid]->SetClientInfo (resource, pres);
		JID2CLEntry_ [jid]->SetStatus (PresenceToStatus (pres), resource);
	}
	
	void ClientConnection::handleRoomPresenceChanged (const QString& room,
			const QString& nick, const QXmppPresence& pres)
	{
		if (!RoomHandlers_.contains (room))
		{
			qWarning () << Q_FUNC_INFO
					<< "no room handler for"
					<< room
					<< nick;
			return;
		}
		
		RoomHandlers_ [room]->HandlePresence (pres, nick);
	}

	void ClientConnection::handleMessageReceived (const QXmppMessage& msg)
	{
		if (msg.type () == QXmppMessage::Error)
		{
			qDebug () << Q_FUNC_INFO
					<< "got error message from"
					<< msg.from ();
			return;
		}

		QString jid;
		QString resource;
		Split (msg.from (), &jid, &resource);

		if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->HandleMessage (msg, resource);
		else if (JID2CLEntry_.contains (jid))
		{
			if (msg.state ())
				JID2CLEntry_ [jid]->UpdateChatState (msg.state (), resource);

			if (!msg.body ().isEmpty ())
			{
				GlooxMessage *gm = new GlooxMessage (msg, this);
				JID2CLEntry_ [jid]->HandleMessage (gm);
			}
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
	
	void ClientConnection::handleRoomPartNickChange (const QString& roomJid,
			const QString& oldNick, const QString& newNick)
	{
		if (!RoomHandlers_.contains (roomJid))
		{
			qWarning () << Q_FUNC_INFO
					<< "no RoomHandler for"
					<< roomJid
					<< RoomHandlers_.keys ();
			return;
		}
		
		RoomHandlers_ [roomJid]->HandleNickChange (oldNick, newNick);
	}
	
	void ClientConnection::handleRoomParticipantPermsChanged (const QString& roomJid,
			const QString& nick, QXmppMucAdminIq::Item::Affiliation aff,
			QXmppMucAdminIq::Item::Role role, const QString& reason)
	{
		if (!RoomHandlers_.contains (roomJid))
		{
			qWarning () << Q_FUNC_INFO
					<< "no RoomHandler for"
					<< roomJid
					<< RoomHandlers_.keys ();
			return;
		}
		
		RoomHandlers_ [roomJid]->HandlePermsChanged (nick, aff, role, reason);
	}
	
	void ClientConnection::decrementErrAccumulators ()
	{
		if (SocketErrorAccumulator_ > 0)
			--SocketErrorAccumulator_;
	}

	/** @todo Handle action reasons in QXmppPresence::Subscribe and
	 * QXmppPresence::Unsubscribe cases.
	 */
	void ClientConnection::HandleOtherPresence (const QXmppPresence& pres)
	{
		qDebug () << "OtherPresence" << pres.from () << pres.type ();
		const QString& jid = pres.from ();
		switch (pres.type ())
		{
		case QXmppPresence::Subscribe:
			emit gotSubscriptionRequest (new UnauthCLEntry (jid, QString (), Account_),
					QString ());
			break;
		case QXmppPresence::Subscribed:
			if (JID2CLEntry_.contains (jid))
				emit rosterItemGrantedSubscription (JID2CLEntry_ [jid], QString ());
			break;
		case QXmppPresence::Unsubscribe:
			if (JID2CLEntry_.contains (jid))
				emit rosterItemUnsubscribed (JID2CLEntry_ [jid], QString ());
			else
				emit rosterItemUnsubscribed (jid, QString ());
			break;
		case QXmppPresence::Unsubscribed:
			if (JID2CLEntry_.contains (jid))
				emit rosterItemCancelledSubscription (JID2CLEntry_ [jid], QString ());
			break;
		}
	}

	void ClientConnection::HandleError (const QXmppIq& iq)
	{
		const QXmppStanza::Error& error = iq.error ();
		if (error.condition () == QXmppStanza::Error::FeatureNotImplemented)
		{
			// Whatever it is, it just keeps appearing, hz.
			return;
		}
		QString typeText;
		if (!iq.from ().isEmpty ())
			typeText = tr ("Error from %1: ")
					.arg (iq.from ());
		typeText += HandleErrorCondition (error.condition ());

		if (!error.text ().isEmpty ())
			typeText += " " + tr ("Error text: %1.")
					.arg (error.text ());

		const Entity& e = Util::MakeNotification ("Azoth",
				typeText,
				PCritical_);
		Core::Instance ().SendEntity (e);
	}

	QString ClientConnection::HandleErrorCondition (const QXmppStanza::Error::Condition& condition)
	{
		switch (condition)
		{
		case QXmppStanza::Error::BadRequest:
			return tr ("Bad request.");
		case QXmppStanza::Error::Conflict:
			return tr ("Conflict (possibly, resource conflict).");
		case QXmppStanza::Error::FeatureNotImplemented:
			return tr ("Feature not implemented.");
		case QXmppStanza::Error::Forbidden:
			return tr ("Forbidden.");
			//case QXmppStanza::Error::Gone:
		case QXmppStanza::Error::InternalServerError:
			return tr ("Internal server error.");
		case QXmppStanza::Error::ItemNotFound:
			return tr ("Item not found.");
		case QXmppStanza::Error::JidMalformed:
			return tr ("JID is malformed.");
		case QXmppStanza::Error::NotAcceptable:
			return tr ("Data is not acceptable.");
		case QXmppStanza::Error::NotAllowed:
			return tr ("Action is not allowed.");
		case QXmppStanza::Error::NotAuthorized:
			emit serverAuthFailed ();
			return tr ("Not authorized.");
		case QXmppStanza::Error::PaymentRequired:
			return tr ("Payment required.");
		case QXmppStanza::Error::RecipientUnavailable:
			return tr ("Recipient unavailable.");
		case QXmppStanza::Error::Redirect:
			return tr ("Got redirect.");
		case QXmppStanza::Error::RegistrationRequired:
			return tr ("Registration required.");
		case QXmppStanza::Error::RemoteServerNotFound:
			return tr ("Remote server not found.");
		case QXmppStanza::Error::RemoteServerTimeout:
			return tr ("Timeout contacting remote server.");
		case QXmppStanza::Error::ResourceConstraint:
			return tr ("Error due to resource constraint.");
		case QXmppStanza::Error::ServiceUnavailable:
			return tr ("Service is unavailable at the moment.");
		case QXmppStanza::Error::SubscriptionRequired:
			return tr ("Subscription is required to perform this action.");
			//case QXmppStanza::Error::UndefinedCondition:
			//case QXmppStanza::Error::UnexpectedRequest:
		default:
			return tr ("Other error.");
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
	*/

	GlooxCLEntry* ClientConnection::CreateCLEntry (const QString& jid)
	{
		return CreateCLEntry (Client_->rosterManager ().getRosterEntry (jid));
	}

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
