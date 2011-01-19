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

		/*
		Client_->disco ()->setIdentity ("client", "pc", "LeechCraft Azoth");
		Client_->disco ()->addFeature (gloox::XMLNS_ROSTER);
		Client_->disco ()->addFeature (gloox::XMLNS_COMPRESSION);
		Client_->disco ()->addFeature (gloox::XMLNS_STREAM_COMPRESS);

		gloox::Capabilities *caps = new gloox::Capabilities (Client_->disco ());
		caps->setNode ("http://leechcraft.org/azoth");
		Client_->addPresenceExtension (caps);
		*/
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
		rh->SetState (LastState_);

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

	void ClientConnection::Update (const QXmppRosterIq::Item& item)
	{
		QXmppRosterIq iq;
		iq.addItem (item);
		Client_->sendPacket (iq);
	}

	void ClientConnection::Update (const QXmppMucAdminIq::Item& item)
	{
		QXmppMucAdminIq iq;
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

		QXmppRosterIq::Item item;
		item.setBareJid (entry->GetJID ());
		item.setSubscriptionStatus (ack ? "subscribe" : "unsubscribe");
		item.setSubscriptionType (QXmppRosterIq::Item::Both);
		Update (item);

		emit rosterItemRemoved (entry);
		entry->deleteLater ();
	}

	void ClientConnection::Subscribe (const QString& id,
			const QString& msg, const QString& name, const QStringList& groups)
	{
		QXmppRosterIq::Item item;
		item.setBareJid (id);
		item.setGroups (QSet<QString>::fromList (groups));
		item.setName (name);
		item.setSubscriptionType (QXmppRosterIq::Item::To);
		Update (item);
	}

	void ClientConnection::RevokeSubscription (const QString& jid, const QString& reason)
	{
		QXmppRosterIq::Item item = Client_->rosterManager ().getRosterEntry (jid);
		if (item.subscriptionType () == QXmppRosterIq::Item::None ||
				item.subscriptionType () == QXmppRosterIq::Item::From)
			return;

		QXmppRosterIq::Item::SubscriptionType newSub = QXmppRosterIq::Item::None;
		if (item.subscriptionType () == QXmppRosterIq::Item::Both)
			newSub = QXmppRosterIq::Item::To;
		item.setSubscriptionType (newSub);
		if (!reason.isEmpty ())
			item.setSubscriptionStatus (reason);

		Update (item);
	}

	void ClientConnection::Unsubscribe (const QString& jid, const QString& reason)
	{
	}

	void ClientConnection::Remove (GlooxCLEntry *entry)
	{
		QXmppRosterIq::Item item = entry->GetRI ();
		item.setSubscriptionType (QXmppRosterIq::Item::Remove);
		Update (item);
	}

	QXmppClient* ClientConnection::GetClient () const
	{
		return Client_;
	}

	GlooxCLEntry* ClientConnection::GetCLEntry (const QString& bareJid) const
	{
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

	void ClientConnection::handleConnected ()
	{
		IsConnected_ = true;
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

	void ClientConnection::onDisconnect (gloox::ConnectionError e)
	{
		IsConnected_ = false;

		if (e == gloox::ConnNoError)
			return;

		QString error;
		switch (e)
		{
		case gloox::ConnStreamError:
			error = tr ("stream error: %1")
					.arg (QString::fromUtf8 (Client_->
									streamErrorText ().c_str ()));
			break;
		case gloox::ConnStreamVersionError:
			error = tr ("stream version not supported");
			break;
		case gloox::ConnStreamClosed:
			error = tr ("stream has been closed by the server");
			break;
		case gloox::ConnProxyAuthRequired:
			error = tr ("proxy server requires authentication");
			break;
		case gloox::ConnProxyAuthFailed:
			error = tr ("proxy server authentication failed");
			break;
		case gloox::ConnProxyNoSupportedAuth:
			error = tr ("proxy server requires an unsupported authentication method");
			break;
		case gloox::ConnIoError:
			error = tr ("an I/O error occured");
			break;
		case gloox::ConnParseError:
			error = tr ("XML parse error occured");
			break;
		case gloox::ConnConnectionRefused:
			error = tr ("connection was refused by the server");
			break;
		case gloox::ConnDnsError:
			error = tr ("resolving the server's hostname failed");
			break;
		case gloox::ConnOutOfMemory:
			error = tr ("out of memory");
			break;
		case gloox::ConnNoSupportedAuth:
			error = tr ("authentication mechanisms offered by the server are not supported");
			break;
		case gloox::ConnTlsFailed:
			error = tr ("server's certificate could not be verified or TLS handshake failed");
			break;
		case gloox::ConnTlsNotAvailable:
			error = tr ("server didn't offer TLS");
			break;
		case gloox::ConnCompressionFailed:
			error = tr ("initializing compression failed");
			break;
		case gloox::ConnAuthenticationFailed:
			error = tr ("authentication failed, %1");
			break;
		case gloox::ConnUserDisconnected:
			error = tr ("user disconnect requested");
			break;
		case gloox::ConnNotConnected:
			error = tr ("no active connection");
			break;
		}

		if (e == gloox::ConnAuthenticationFailed)
		{
			QString ae;
			switch (Client_->authError ())
			{
				case gloox::AuthErrorUndefined:
					ae = tr ("error condition is unknown");
					break;
				case gloox::SaslAborted:
					ae = tr ("SASL aborted");
					break;
				case gloox::SaslIncorrectEncoding:
					ae = tr ("incorrect encoding");
					break;
				case gloox::SaslInvalidAuthzid:
					ae = tr ("authzid provided by initiating entity is invalid");
					break;
				case gloox::SaslInvalidMechanism:
					ae = tr ("initiating entity provided a mechanism not supported by the receiving entity");
					break;
				case gloox::SaslMalformedRequest:
					ae = tr ("malformed request");
					break;
				case gloox::SaslMechanismTooWeak:
					ae = tr ("mechanism requested by initiating entity is weaker than server policy permits");
					break;
				case gloox::SaslNotAuthorized:
				case gloox::NonSaslNotAuthorized:
					ae = tr ("initiating entity did not provide valid credentials");
					emit serverAuthFailed ();
					break;
				case gloox::SaslTemporaryAuthFailure:
					ae = tr ("temporary error withing receiving entity");
					break;
				case gloox::NonSaslConflict:
					ae = tr ("resource conflict");
					break;
				case gloox::NonSaslNotAcceptable:
					ae = tr ("required information not provided");
					break;
			}
			error = error.arg (ae);
		}

		QString message = tr ("Disconnected, %1.")
				.arg (error);
		qWarning () << Q_FUNC_INFO << message;

		Entity e = Util::MakeNotification (tr ("Azoth connection error"),
				message,
				PCritical_);
		Core::Instance ().SendEntity (e);
	}

	void ClientConnection::onResourceBind (const std::string& resource)
	{
		qDebug () << Q_FUNC_INFO << resource.c_str ();
	}

	void ClientConnection::onResourceBindError (const gloox::Error *error)
	{
		qWarning () << Q_FUNC_INFO;
		if (error)
			qWarning () << error->text ().c_str ();
	}

	void ClientConnection::onSessionCreateError (const gloox::Error *error)
	{
		qWarning () << Q_FUNC_INFO;
		if (error)
			qWarning () << error->text ().c_str ();
	}

	void ClientConnection::onStreamEvent (gloox::StreamEvent e)
	{
		qDebug () << Q_FUNC_INFO;
	}

	bool ClientConnection::onTLSConnect (const gloox::CertInfo& info)
	{
		qDebug () << Q_FUNC_INFO << info.server.c_str ();
		return true;
	}

	void ClientConnection::handlePollTimer ()
	{
		Client_->recv (1000);
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

	void ClientConnection::handleItemUnsubscribed (const gloox::JID& jid)
	{
		qDebug () << Q_FUNC_INFO << jid.full ().c_str ();
		// TODO
	}

	void ClientConnection::handleRoster (const gloox::Roster& roster)
	{
		QList<QObject*> entries;
		for (gloox::Roster::const_iterator i = roster.begin (),
				end = roster.end (); i != end; ++i)
		{
			GlooxCLEntry *entry = CreateCLEntry (i->second);
			entries << entry;
		}

		if (entries.size ())
			emit gotRosterItems (entries);
	}

	void ClientConnection::handleRosterPresence (const gloox::RosterItem& item,
				const std::string& resource,
				gloox::Presence::PresenceType type,
				const std::string& msg)
	{
		gloox::JID jid (item.jid ());
		if (!JID2CLEntry_.contains (jid))
		{
			if (ODSEntries_.contains (jid))
				ConvertFromODS (jid.bareJID (), Client_->rosterManager ()->getRosterItem (jid));
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "no GlooxCLEntry for item"
						<< item.jid ().c_str ();
				return;
			}
		}

		GlooxCLEntry *entry = JID2CLEntry_ [jid];

		EntryStatus status (static_cast<State> (type),
				QString::fromUtf8 (msg.c_str ()));

		entry->SetStatus (status, QString::fromUtf8 (resource.c_str ()));
	}

	void ClientConnection::handleSelfPresence (const gloox::RosterItem& item,
				const std::string& resource,
				gloox::Presence::PresenceType type,
				const std::string& msg)
	{
		// TODO
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

	bool ClientConnection::handleUnsubscriptionRequest (const gloox::JID&, const std::string&)
	{
		qDebug () << Q_FUNC_INFO;
		// TODO
		return false;
	}

	void ClientConnection::handleNonrosterPresence (const gloox::Presence&)
	{
		qDebug () << Q_FUNC_INFO;
		// TODO
	}

	void ClientConnection::handleRosterError (const gloox::IQ&)
	{
		qDebug () << Q_FUNC_INFO;
		// TODO
	}

	void ClientConnection::handleMessageSession (gloox::MessageSession *session)
	{
		gloox::JID jid = session->target ();
		gloox::JID bareJid = jid.bareJID ();
		QString resource = QString::fromUtf8 (jid.resource ().c_str ());
		if (!Sessions_ [bareJid].contains (resource) ||
				Sessions_ [bareJid] [resource] != session)
			Sessions_ [bareJid] [resource] = session;

		session->registerMessageHandler (this);
	}

	void ClientConnection::handleMessage (const gloox::Message& msg, gloox::MessageSession *session)
	{
		gloox::JID jid = session->target ().bareJID ();

		if (!JID2CLEntry_.contains (jid))
		{
			qWarning () << Q_FUNC_INFO
					<< "map doesn't contain"
					<< QString::fromUtf8 (jid.full ().c_str());
			return;
		}

		GlooxCLEntry *entry = JID2CLEntry_ [jid];
		GlooxMessage *gm = new GlooxMessage (msg, entry, session);
		gm->SetDateTime (QDateTime::currentDateTime ());

		entry->HandleMessage (gm);
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

	void ClientConnection::handleVCardResult (gloox::VCardHandler::VCardContext ctx,
			const gloox::JID& jid, gloox::StanzaError er)
	{
		// TODO
	}

	void ClientConnection::handlePresence (const gloox::Presence& presence)
	{
		const gloox::Capabilities *caps = presence.capabilities ();
		if (!caps)
			return;

		const gloox::JID& from = presence.from ();
		if (JID2CLEntry_.contains (from.bareJID ()))
		{
			const QString& var = QString::fromUtf8 (from.resource ().c_str ());
			JID2CLEntry_ [from.bareJID ()]->SetClientInfo (var, caps);
		}
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
