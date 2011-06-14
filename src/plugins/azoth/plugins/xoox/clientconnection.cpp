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

#include "clientconnection.h"
#include <boost/bind.hpp>
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
#include <QXmppBookmarkManager.h>
#include <QXmppEntityTimeManager.h>
#include <QXmppArchiveManager.h>
#include <QXmppPubSubManager.h>
#include <QXmppActivityItem.h>
#include <QXmppPubSubIq.h>
#include <QXmppDeliveryReceiptsManager.h>
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
#include "vcarddialog.h"
#include "capsmanager.h"
#include "annotationsmanager.h"
#include "fetchqueue.h"

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
	: Client_ (new QXmppClient (this))
	, MUCManager_ (new QXmppMucManager)
	, XferManager_ (new QXmppTransferManager)
	, DiscoveryManager_ (Client_->findExtension<QXmppDiscoveryManager> ())
	, BMManager_ (new QXmppBookmarkManager (Client_))
	, EntityTimeManager_ (new QXmppEntityTimeManager)
	, ArchiveManager_ (new QXmppArchiveManager)
	, PubSubManager_ (new QXmppPubSubManager)
	, DeliveryReceiptsManager_ (new QXmppDeliveryReceiptsManager)
	, AnnotationsManager_ (0)
	, OurJID_ (jid)
	, Account_ (account)
	, ProxyObject_ (0)
	, CapsManager_ (new CapsManager (this))
	, IsConnected_ (false)
	, FirstTimeConnect_ (true)
	, VCardQueue_ (new FetchQueue (boost::bind (&QXmppVCardManager::requestVCard, &Client_->vCardManager (), _1),
				1700, 1, this))
	, CapsQueue_ (new FetchQueue (boost::bind (&QXmppDiscoveryManager::requestInfo, DiscoveryManager_, _1, ""),
				1000, 1, this))
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

		Client_->addExtension (DeliveryReceiptsManager_);
		Client_->addExtension (MUCManager_);
		Client_->addExtension (XferManager_);
		Client_->addExtension (BMManager_);
		Client_->addExtension (EntityTimeManager_);
		Client_->addExtension (ArchiveManager_);
		Client_->addExtension (PubSubManager_);
		
		AnnotationsManager_ = new AnnotationsManager (this);

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
				SIGNAL (itemAdded (const QString&)),
				this,
				SLOT (handleRosterChanged (const QString&)));
		connect (&Client_->rosterManager (),
				SIGNAL (itemChanged (const QString&)),
				this,
				SLOT (handleRosterChanged (const QString&)));
		connect (&Client_->rosterManager (),
				SIGNAL (itemAdded (const QString&)),
				&Core::Instance (),
				SLOT (saveRoster ()),
				Qt::QueuedConnection);
		connect (&Client_->rosterManager (),
				SIGNAL (itemRemoved (const QString&)),
				this,
				SLOT (handleRosterItemRemoved (const QString&)));

		connect (&Client_->vCardManager (),
				SIGNAL (vCardReceived (const QXmppVCardIq&)),
				this,
				SLOT (handleVCardReceived (const QXmppVCardIq&)));
		
		connect (DeliveryReceiptsManager_,
				SIGNAL (messageDelivered (const QString&)),
				this,
				SLOT (handleMessageDelivered (const QString&)));

		connect (DiscoveryManager_,
				SIGNAL (infoReceived (const QXmppDiscoveryIq&)),
				CapsManager_,
				SLOT (handleInfoReceived (const QXmppDiscoveryIq&)));
		connect (DiscoveryManager_,
				SIGNAL (itemsReceived (const QXmppDiscoveryIq&)),
				CapsManager_,
				SLOT (handleItemsReceived (const QXmppDiscoveryIq&)));
		connect (DiscoveryManager_,
				SIGNAL (infoReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleDiscoInfo (const QXmppDiscoveryIq&)));
		connect (DiscoveryManager_,
				SIGNAL (itemsReceived (const QXmppDiscoveryIq&)),
				this,
				SLOT (handleDiscoItems (const QXmppDiscoveryIq&)));
		
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
		if (presType != QXmppPresence::Unavailable)
			Q_FOREACH (RoomHandler *rh, RoomHandlers_)
				rh->SetState (state);
		else
			Q_FOREACH (RoomHandler *rh, RoomHandlers_)
				rh->Leave (QString (), false);

		if (!IsConnected_ &&
				state.State_ != SOffline)
		{
			emit statusChanged (EntryStatus (SConnecting, QString ()));
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
				GlooxCLEntry *entry = JID2CLEntry_ [jid];
				Q_FOREACH (const QString& var, entry->Variants ())
					entry->SetStatus (EntryStatus (SOffline, QString ()), var);
				JID2CLEntry_.remove (jid);
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
		
		if (!JoinQueue_.isEmpty ())
		{
			QList<JoinQueueItem>::iterator pos = std::find_if (JoinQueue_.begin (), JoinQueue_.end (),
					boost::bind (std::equal_to<QString> (), jid,
						boost::bind (&JoinQueueItem::RoomJID_, _1)));
			if (pos != JoinQueue_.end ())
				JoinQueue_.erase (pos);
		}

		RoomHandler *rh = new RoomHandler (jid, nick, Account_);
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
	
	QXmppDiscoveryManager* ClientConnection::GetDiscoveryManager () const
	{
		return DiscoveryManager_;
	}

	QXmppTransferManager* ClientConnection::GetTransferManager () const
	{
		return XferManager_;
	}
	
	CapsManager* ClientConnection::GetCapsManager () const
	{
		return CapsManager_;
	}
	
	AnnotationsManager* ClientConnection::GetAnnotationsManager () const
	{
		return AnnotationsManager_;
	}

	void ClientConnection::RequestInfo (const QString& jid) const
	{
		if (JID2CLEntry_.contains (jid))
			Q_FOREACH (const QString& variant, JID2CLEntry_ [jid]->Variants ())
				CapsQueue_->Schedule (jid + '/' + variant, FetchQueue::PHigh);
		else
			CapsQueue_->Schedule (jid, FetchQueue::PLow);
	}
	
	void ClientConnection::RequestInfo (const QString& jid,
			DiscoCallback_t callback, const QString& node)
	{
		AwaitingDiscoInfo_ [jid] = callback;
		
		DiscoveryManager_->requestInfo (jid, node);
	}
	
	void ClientConnection::RequestItems (const QString& jid,
			DiscoCallback_t callback, const QString& node)
	{
		AwaitingDiscoItems_ [jid] = callback;
		
		DiscoveryManager_->requestItems (jid, node);
	}

	void ClientConnection::Update (const QXmppRosterIq::Item& item)
	{
		QXmppRosterIq iq;
		iq.setType (QXmppIq::Set);
		iq.addItem (item);
		Client_->sendPacket (iq);
	}

	void ClientConnection::Update (const QXmppMucItem& item, const QString& room)
	{
		QXmppMucAdminIq iq;
		iq.setTo (room);
		iq.setType (QXmppIq::Set);
		iq.setItems (QList<QXmppMucItem> () << item);
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
			Client_->rosterManager ().acceptSubscription (jid);
			Subscribe (jid, QString (), entry->GetEntryName (), entry->Groups ());
		}
		else
			Client_->rosterManager ().refuseSubscription (jid);

		emit rosterItemRemoved (entry);
		entry->deleteLater ();
	}
	
	void ClientConnection::AddEntry (const QString& id,
			const QString& name, const QStringList& groups)
	{
		Client_->rosterManager ().addRosterEntry (id,
				name, QString (), QSet<QString>::fromList (groups));
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
		Client_->rosterManager ().refuseSubscription (jid, reason);
	}

	void ClientConnection::Unsubscribe (const QString& jid, const QString& reason)
	{
		qDebug () << "Unsubscribe" << jid;
		Client_->rosterManager ().unsubscribe (jid, reason);
	}

	void ClientConnection::Remove (GlooxCLEntry *entry)
	{
		const QString& jid = entry->GetJID ();

		Client_->rosterManager ().removeItem (jid);

		if (ODSEntries_.contains (jid))
			delete ODSEntries_.take (jid);
	}
	
	void ClientConnection::SendPacketWCallback (const QXmppIq& packet,
			QObject *obj, const QByteArray& method)
	{
		AwaitingPacketCallbacks_ [packet.to ()] [packet.id ()] = PacketCallback_t (obj, method);
		Client_->sendPacket (packet);
	}
	
	void ClientConnection::SendMessage (GlooxMessage *msgObj)
	{
		const QXmppMessage& msg = msgObj->GetMessage ();
		if (msg.requestReceipt ())
			UndeliveredMessages_ [msg.id ()] = msgObj;
		
		Client_->sendPacket (msg);
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
		ODSEntries_ [entry->GetJID ()] = entry;

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
		ScheduleFetchVCard (jid);
	}
	
	void ClientConnection::FetchVCard (const QString& jid, VCardDialog *dia)
	{
		AwaitingVCardDialogs_ [jid] = QPointer<VCardDialog> (dia);
		FetchVCard (jid);
	}
	
	QXmppBookmarkSet ClientConnection::GetBookmarks () const
	{
		return BMManager_->bookmarks ();
	}
	
	void ClientConnection::SetBookmarks (const QXmppBookmarkSet& set)
	{
		BMManager_->setBookmarks (set);
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
		if (bare)
			*bare = jid.left (pos);
		if (resource)
			*resource = (pos >= 0 ? jid.mid (pos + 1) : QString ());
	}

	void ClientConnection::handleConnected ()
	{
		IsConnected_ = true;
		emit statusChanged (EntryStatus (LastState_.State_, LastState_.Status_));
		
		connect (BMManager_,
				SIGNAL (bookmarksReceived (const QXmppBookmarkSet&)),
				this,
				SLOT (handleBookmarksReceived (const QXmppBookmarkSet&)),
				Qt::UniqueConnection);
		
		AnnotationsManager_->refetchNotes ();
		
		Q_FOREACH (RoomHandler *rh, RoomHandlers_)
			rh->Join ();
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
		
		try
		{
			dynamic_cast<const QXmppActivityItem&> (iq);
			qDebug () << "got activity item" << iq.id ();
		}
		catch (...)
		{
		}
		
		try
		{
			dynamic_cast<const QXmppPubSubIq&> (iq);
			qDebug () << "got pubsub item" << iq.id ();
		}
		catch (...)
		{
		}
		
		InvokeCallbacks (iq);
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
		
		Q_FOREACH (const QXmppMessage& msg, OfflineMsgQueue_)
			handleMessageReceived (msg);

		OfflineMsgQueue_.clear ();
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
		
		if (AwaitingVCardDialogs_.contains (jid))
		{
			QPointer<VCardDialog> dia = AwaitingVCardDialogs_ [jid];
			if (dia)
				dia->UpdateInfo (vcard);
			AwaitingVCardDialogs_.remove (jid);
		}

		if (JID2CLEntry_.contains (jid))
			JID2CLEntry_ [jid]->SetVCard (vcard);
		else if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->GetParticipantEntry (nick)->SetVCard (vcard);
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
			
			if (msg.isAttention ())
				JID2CLEntry_ [jid]->HandleAttentionMessage (msg);
		}
		else if (!Client_->rosterManager ().isRosterReceived ())
			OfflineMsgQueue_ << msg;
		else
			qWarning () << Q_FUNC_INFO
					<< "could not find source for"
					<< msg.from ();
	}
	
	void ClientConnection::handleMessageDelivered (const QString& msgId)
	{
		QPointer<GlooxMessage> msg = UndeliveredMessages_.take (msgId);
		if (msg)
			msg->SetDelivered (true);
	}

	void ClientConnection::handleBookmarksReceived (const QXmppBookmarkSet& set)
	{
		disconnect (BMManager_,
				SIGNAL (bookmarksReceived (const QXmppBookmarkSet&)),
				this,
				SLOT (handleBookmarksReceived (const QXmppBookmarkSet&)));

		Q_FOREACH (const QXmppBookmarkConference& conf, set.conferences ())
		{
			if (!conf.autoJoin ())
				continue;
			
			JoinQueueItem item =
			{
				conf.jid (),
				conf.nickName ()
			};
			JoinQueue_ << item;
		}
		
		if (JoinQueue_.size ())
			QTimer::singleShot (10000,
					this,
					SLOT (handleAutojoinQueue ()));
	}
	
	void ClientConnection::handleAutojoinQueue ()
	{
		if (JoinQueue_.isEmpty ())
			return;

		GlooxProtocol *proto =
				qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		if (!qobject_cast<IProxyObject*> (proto->GetProxyObject ())->IsAutojoinAllowed ())
			return;

		QList<QObject*> entries;
		Q_FOREACH (const JoinQueueItem& item, JoinQueue_)
			entries << JoinRoom (item.RoomJID_, item.Nickname_);
		emit gotRosterItems (entries);
		JoinQueue_.clear ();
	}
	
	void ClientConnection::handleDiscoInfo (const QXmppDiscoveryIq& iq)
	{
		const QString& jid = iq.from ();
		if (AwaitingDiscoInfo_.contains (jid))
			AwaitingDiscoInfo_ [jid] (iq);
	}
	
	void ClientConnection::handleDiscoItems (const QXmppDiscoveryIq& iq)
	{
		const QString& jid = iq.from ();
		if (AwaitingDiscoItems_.contains (jid))
			AwaitingDiscoItems_ [jid] (iq);
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
		case QXmppPresence::Error:
		{
			QString bare;
			QString resource;
			ClientConnection::Split (jid, &bare, &resource);
			if (RoomHandlers_.contains (bare))
				RoomHandlers_ [bare]->HandleErrorPresence (pres, resource);;
			break;
		}
		case QXmppPresence::Available:
		case QXmppPresence::Unavailable:
		case QXmppPresence::Probe:
			qWarning () << Q_FUNC_INFO
					<< "got wrong presence"
					<< pres.type ();
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
	
	void ClientConnection::InvokeCallbacks (const QXmppIq& iq)
	{
		if (!AwaitingPacketCallbacks_.contains (iq.from ()))
			return;
		
		const PacketID2Callback_t& cbs = AwaitingPacketCallbacks_ [iq.from ()];
		if (!cbs.contains (iq.id ()))
			return;
	
		const PacketCallback_t& cb = cbs [iq.id ()];
		if (!cb.first)
			return;
		
		QMetaObject::invokeMethod (cb.first,
				cb.second,
				Q_ARG (QXmppIq, iq));
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

	void ClientConnection::ScheduleFetchVCard (const QString& jid)
	{
		FetchQueue::Priority prio = !JID2CLEntry_.contains (jid) ||
					JID2CLEntry_ [jid]->GetStatus (QString ()).State_ == SOffline ?
				FetchQueue::PLow :
				FetchQueue::PHigh;
		VCardQueue_->Schedule (jid, prio);
	}

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
				ScheduleFetchVCard (bareJID);
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
		{
			const QXmppVCardIq& vcard = entry->GetVCard ();
			if (vcard.nickName ().isEmpty () &&
					vcard.birthday ().isNull () &&
					vcard.email ().isEmpty () &&
					vcard.firstName ().isEmpty () &&
					vcard.lastName ().isEmpty ())
				ScheduleFetchVCard (bareJID);
		}
		return entry;
	}
}
}
}
