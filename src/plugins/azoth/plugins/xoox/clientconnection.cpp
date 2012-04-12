/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QXmppBookmarkManager.h>
#include <QXmppEntityTimeManager.h>
#include <QXmppArchiveManager.h>
#include <QXmppActivityItem.h>
#include <QXmppPubSubIq.h>
#include <QXmppDeliveryReceiptsManager.h>
#include <QXmppCaptchaManager.h>
#include <QXmppBobManager.h>
#include <QXmppCallManager.h>
#include <util/util.h>
#include <util/socketerrorstrings.h>
#include <util/sysinfo.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxaccount.h"
#include "glooxclentry.h"
#include "glooxmessage.h"
#include "roomhandler.h"
#include "glooxprotocol.h"
#include "core.h"
#include "roomclentry.h"
#include "vcarddialog.h"
#include "capsmanager.h"
#include "annotationsmanager.h"
#include "formbuilder.h"
#include "fetchqueue.h"
#include "legacyentitytimeext.h"
#include "pubsubmanager.h"
#include "useractivity.h"
#include "usermood.h"
#include "usertune.h"
#include "userlocation.h"
#include "privacylistsmanager.h"
#include "adhoccommandmanager.h"
#include "util.h"
#include "selfcontact.h"
#include "adhoccommandserver.h"
#include "lastactivitymanager.h"
#include "jabbersearchmanager.h"
#include "useravatarmanager.h"
#include "msgarchivingmanager.h"
#include "sdmanager.h"

#ifdef ENABLE_CRYPT
#include "pgpmanager.h"
#include "xep0232handler.h"
#endif

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const int ErrorLimit = 5;

	ClientConnection::ClientConnection (const QString& jid,
			GlooxAccount *account)
	: Client_ (new QXmppClient (this))
	, MUCManager_ (new QXmppMucManager)
	, XferManager_ (new QXmppTransferManager)
	, DiscoveryManager_ (Client_->findExtension<QXmppDiscoveryManager> ())
	, BMManager_ (new QXmppBookmarkManager (Client_))
	, EntityTimeManager_ (Client_->findExtension<QXmppEntityTimeManager> ())
	, ArchiveManager_ (new QXmppArchiveManager)
	, DeliveryReceiptsManager_ (new QXmppDeliveryReceiptsManager)
	, CaptchaManager_ (new QXmppCaptchaManager)
	, BobManager_ (new QXmppBobManager)
#ifdef ENABLE_MEDIACALLS
	, CallManager_ (new QXmppCallManager)
#endif
	, PubSubManager_ (new PubSubManager)
	, PrivacyListsManager_ (new PrivacyListsManager)
	, AdHocCommandManager_ (new AdHocCommandManager (this))
	, AnnotationsManager_ (0)
	, LastActivityManager_ (new LastActivityManager)
	, JabberSearchManager_ (new JabberSearchManager)
	, UserAvatarManager_ (0)
	, RIEXManager_ (new RIEXManager)
	, MsgArchivingManager_ (new MsgArchivingManager (this))
	, SDManager_ (new SDManager (this))
#ifdef ENABLE_CRYPT
	, PGPManager_ (0)
#endif
	, OurJID_ (jid)
	, SelfContact_ (new SelfContact (jid, account))
	, Account_ (account)
	, ProxyObject_ (0)
	, CapsManager_ (new CapsManager (this))
	, IsConnected_ (false)
	, FirstTimeConnect_ (true)
	, VCardQueue_ (new FetchQueue ([this] (QString str) { Client_->vCardManager ().requestVCard (str); },
				jid.contains ("gmail.com") ? 1700 : 300, 1, this))
	, CapsQueue_ (new FetchQueue ([this] (QString str) { DiscoveryManager_->requestInfo (str, ""); },
				jid.contains ("gmail.com") ? 1000 : 200, 1, this))
	, VersionQueue_ (new FetchQueue ([this] (QString str) { Client_->versionManager ().requestVersion (str); },
				jid.contains ("gmail.com") ? 1200 : 250, 1, this))
	, SocketErrorAccumulator_ (0)
	, KAInterval_ (90)
	, KATimeout_ (30)
	{
		SetOurJID (OurJID_);

		SetupLogger ();

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

		PubSubManager_->RegisterCreator<UserActivity> ();
		PubSubManager_->RegisterCreator<UserMood> ();
		PubSubManager_->RegisterCreator<UserTune> ();
		PubSubManager_->RegisterCreator<UserLocation> ();
		PubSubManager_->SetAutosubscribe<UserActivity> (true);
		PubSubManager_->SetAutosubscribe<UserMood> (true);
		PubSubManager_->SetAutosubscribe<UserTune> (true);
		PubSubManager_->SetAutosubscribe<UserLocation> (true);

		connect (PubSubManager_,
				SIGNAL (gotEvent (const QString&, PEPEventBase*)),
				this,
				SLOT (handlePEPEvent (const QString&, PEPEventBase*)));

		UserAvatarManager_ = new UserAvatarManager (this);
		connect (UserAvatarManager_,
				SIGNAL (avatarUpdated (QString, QImage)),
				this,
				SLOT (handlePEPAvatarUpdated (QString, QImage)));

		InitializeQCA ();

		Client_->addExtension (BobManager_);
		Client_->addExtension (PubSubManager_);
		Client_->addExtension (DeliveryReceiptsManager_);
		Client_->addExtension (MUCManager_);
		Client_->addExtension (XferManager_);
		Client_->addExtension (BMManager_);
		Client_->addExtension (ArchiveManager_);
		Client_->addExtension (CaptchaManager_);
		Client_->addExtension (new LegacyEntityTimeExt);
		Client_->addExtension (PrivacyListsManager_);
#ifdef ENABLE_MEDIACALLS
		Client_->addExtension (CallManager_);
#endif
		Client_->addExtension (LastActivityManager_);
		Client_->addExtension (JabberSearchManager_);
		Client_->addExtension (RIEXManager_);
		Client_->addExtension (AdHocCommandManager_);
		Client_->addExtension (new AdHocCommandServer (this));

		AnnotationsManager_ = new AnnotationsManager (this);

		DiscoveryManager_->setClientCapabilitiesNode ("http://leechcraft.org/azoth");

		const auto& sysInfo = Util::SysInfo::GetOSNameSplit ();
		auto& vm = Client_->versionManager ();
		vm.setClientName ("LeechCraft Azoth");
		vm.setClientVersion (Core::Instance ().GetProxy ()->GetVersion ());
		vm.setClientOs (sysInfo.first + ' ' + sysInfo.second);

		XEP0232Handler::SoftwareInformation si =
		{
			64,
			64,
			QUrl ("http://leechcraft.org/leechcraft.png"),
			QString (),
			"image/png",
			sysInfo.first,
			sysInfo.second,
			vm.clientName (),
			vm.clientVersion ()
		};
		DiscoveryManager_->setInfoForm (XEP0232Handler::ToDataForm (si));

		connect (Client_,
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));
		connect (Client_,
				SIGNAL (disconnected ()),
				this,
				SLOT (handleDisconnected ()));
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

		connect (MUCManager_,
				SIGNAL (invitationReceived (QString, QString, QString)),
				this,
				SLOT (handleRoomInvitation (QString, QString, QString)));

		connect (RIEXManager_,
				SIGNAL (gotItems (QString, QList<RIEXManager::Item>, bool)),
				this,
				SLOT (handleGotRIEXItems (QString, QList<RIEXManager::Item>, bool)));

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

		connect (&Client_->versionManager (),
				SIGNAL (versionReceived (const QXmppVersionIq&)),
				this,
				SLOT (handleVersionReceived (const QXmppVersionIq&)));

		connect (DeliveryReceiptsManager_,
				SIGNAL (messageDelivered (const QString&)),
				this,
				SLOT (handleMessageDelivered (const QString&)));

		connect (CaptchaManager_,
				SIGNAL (captchaFormReceived (const QString&, const QXmppDataForm&)),
				this,
				SLOT (handleCaptchaReceived (const QString&, const QXmppDataForm&)));

		connect (BMManager_,
				SIGNAL (bookmarksReceived (QXmppBookmarkSet)),
				Account_,
				SIGNAL (bookmarksChanged ()));

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
		if (!OurPhotoHash_.isEmpty ())
		{
			pres.setVCardUpdateType (QXmppPresence::VCardUpdateValidPhoto);
			pres.setPhotoHash (OurPhotoHash_);
		}

		if (IsConnected_ ||
				state.State_ == SOffline)
			Client_->setClientPresence (pres);

		if (state.State_ == SOffline &&
				!IsConnected_ &&
				!FirstTimeConnect_)
		{
			emit statusChanged (EntryStatus (SOffline, state.Status_));
			emit resetClientConnection ();
			return;
		}

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
			conf.setKeepAliveInterval (KAInterval_);
			conf.setKeepAliveTimeout (KATimeout_);
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
			SelfContact_->RemoveVariant (OurResource_);
		}
	}

	GlooxAccountState ClientConnection::GetLastState () const
	{
		return LastState_;
	}

	QPair<int, int> ClientConnection::GetKAParams () const
	{
		return qMakePair (KAInterval_, KATimeout_);
	}

	void ClientConnection::SetKAParams (const QPair<int, int>& p)
	{
		KAInterval_ = p.first;
		KATimeout_ = p.second;

		if (Client_)
		{
			Client_->configuration ().setKeepAliveInterval (KAInterval_);
			Client_->configuration ().setKeepAliveTimeout (KATimeout_);
		}
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

		Split (jid, &OurBareJID_, &OurResource_);

		SelfContact_->UpdateJID (jid);
	}

	void ClientConnection::SetOurPhotoHash (const QByteArray& hash)
	{
		OurPhotoHash_ = hash;
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
			auto pos = std::find_if (JoinQueue_.begin (), JoinQueue_.end (),
					[&jid] (const JoinQueueItem& it) { return it.RoomJID_ == jid; });
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

	QXmppVersionManager* ClientConnection::GetVersionManager () const
	{
		return &Client_->versionManager ();
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

	PubSubManager* ClientConnection::GetPubSubManager () const
	{
		return PubSubManager_;
	}

	PrivacyListsManager* ClientConnection::GetPrivacyListsManager () const
	{
		return PrivacyListsManager_;
	}

	QXmppBobManager* ClientConnection::GetBobManager () const
	{
		return BobManager_;
	}

#ifdef ENABLE_MEDIACALLS
	QXmppCallManager* ClientConnection::GetCallManager () const
	{
		return CallManager_;
	}
#endif

	AdHocCommandManager* ClientConnection::GetAdHocCommandManager () const
	{
		return AdHocCommandManager_;
	}

	JabberSearchManager* ClientConnection::GetJabberSearchManager () const
	{
		return JabberSearchManager_;
	}

	UserAvatarManager* ClientConnection::GetUserAvatarManager () const
	{
		return UserAvatarManager_;
	}

	RIEXManager* ClientConnection::GetRIEXManager () const
	{
		return RIEXManager_;
	}

	SDManager* ClientConnection::GetSDManager () const
	{
		return SDManager_;
	}

#ifdef ENABLE_CRYPT
	PgpManager* ClientConnection::GetPGPManager () const
	{
		return PGPManager_;
	}

	bool ClientConnection::SetEncryptionEnabled (const QString& jid, bool enabled)
	{
		if (enabled)
			Entries2Crypt_ << jid;
		else
			Entries2Crypt_.remove (jid);

		return true;
	}
#endif

	void ClientConnection::SetSignaledLog (bool signaled)
	{
		if (signaled)
		{
			connect (Client_->logger (),
					SIGNAL (message (QXmppLogger::MessageType, const QString&)),
					this,
					SLOT (handleLog (QXmppLogger::MessageType, const QString&)),
					Qt::UniqueConnection);
			Client_->logger ()->setLoggingType (QXmppLogger::SignalLogging);
		}
		else
		{
			disconnect (Client_->logger (),
					SIGNAL (message (QXmppLogger::MessageType, const QString&)),
					this,
					SLOT (handleLog (QXmppLogger::MessageType, const QString&)));
			Client_->logger ()->setLoggingType (QXmppLogger::FileLogging);
		}
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
		IAuthable *authable = qobject_cast<IAuthable*> (entryObj);
		if (!authable)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "is not authable";
			return;
		}

		if (ack)
		{
			authable->ResendAuth ();
			const AuthStatus status = authable->GetAuthStatus ();
			if (status == ASNone || status == ASFrom)
				authable->RerequestAuth ();
		}
		else
			authable->RevokeAuth ();

		GlooxCLEntry *entry = qobject_cast<GlooxCLEntry*> (entryObj);
		entry->SetAuthRequested (false);
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
		Client_->rosterManager ().acceptSubscription (id, msg);
	}

	void ClientConnection::Unsubscribe (const QString& jid, const QString& reason)
	{
		qDebug () << "Unsubscribe" << jid;
		Client_->rosterManager ().unsubscribe (jid, reason);
	}

	void ClientConnection::GrantSubscription (const QString& jid, const QString& reason)
	{
		qDebug () << "GrantSubscription" << jid;
		Client_->rosterManager ().acceptSubscription (jid, reason);
		if (JID2CLEntry_ [jid])
			JID2CLEntry_ [jid]->SetAuthRequested (false);
	}

	void ClientConnection::RevokeSubscription (const QString& jid, const QString& reason)
	{
		qDebug () << "RevokeSubscription" << jid;
		Client_->rosterManager ().refuseSubscription (jid, reason);
		if (JID2CLEntry_ [jid])
			JID2CLEntry_ [jid]->SetAuthRequested (false);
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
		QXmppMessage msg = msgObj->GetMessage ();
		if (msg.requestReceipt ())
			UndeliveredMessages_ [msg.id ()] = msgObj;

#ifdef ENABLE_CRYPT
		EntryBase *entry = qobject_cast<EntryBase*> (msgObj->OtherPart ());
		if (entry &&
				Entries2Crypt_.contains (entry->GetJID ()))
		{
			const QCA::PGPKey& key = PGPManager_->PublicKey (entry->GetJID ());

			if (!key.isNull ())
			{
				const QString& body = msg.body ();
				msg.setBody (tr ("This message is encrypted. Please decrypt "
								"it to view the original contents."));

				QXmppElement crypt;
				crypt.setTagName ("x");
				crypt.setAttribute ("xmlns", "jabber:x:encrypted");
				crypt.setValue (PGPManager_->EncryptBody (key, body.toUtf8 ()));

				msg.setExtensions (msg.extensions () + QXmppElementList (crypt));
			}
		}
#endif

		Client_->sendPacket (msg);
	}

	QXmppClient* ClientConnection::GetClient () const
	{
		return Client_;
	}

	QObject* ClientConnection::GetCLEntry (const QString& fullJid) const
	{
		QString bare;
		QString variant;
		Split (fullJid, &bare, &variant);

		return GetCLEntry (bare, variant);
	}

	QObject* ClientConnection::GetCLEntry (const QString& bareJid, const QString& variant) const
	{
		if (RoomHandlers_.contains (bareJid))
			return RoomHandlers_ [bareJid]->GetParticipantEntry (variant).get ();
		else if (bareJid == OurBareJID_)
			return SelfContact_;
		else if (JID2CLEntry_.contains (bareJid))
			return JID2CLEntry_ [bareJid];
		else
		{
			QString trueBare, trueVar;
			Split (bareJid, &trueBare, &trueVar);
			if (trueBare != bareJid)
				return GetCLEntry (trueBare, trueVar);
			return 0;
		}
	}

	GlooxCLEntry* ClientConnection::AddODSCLEntry (OfflineDataSource_ptr ods)
	{
		GlooxCLEntry *entry = new GlooxCLEntry (ods, Account_);
		ODSEntries_ [entry->GetJID ()] = entry;

		emit gotRosterItems (QList<QObject*> () << entry);

		return entry;
	}

	QList<QObject*> ClientConnection::GetCLEntries () const
	{
		QList<QObject*> result;
		result << SelfContact_;
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

	void ClientConnection::FetchVCard (const QString& jid, VCardCallback_t callback)
	{
		VCardFetchCallbacks_ [jid] << callback;
		ScheduleFetchVCard (jid);
	}

	void ClientConnection::FetchVersion (const QString& jid)
	{
		VersionQueue_->Schedule (jid);
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
			const QString& resource, const QString& body, const QString& jid)
	{
		GlooxMessage *msg = new GlooxMessage (type,
				IMessage::DOut,
				jid,
				resource,
				this);
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());
		return msg;
	}

	void ClientConnection::SetupLogger ()
	{
		QFile::remove (Util::CreateIfNotExists ("azoth").filePath ("qxmpp.log"));

		QString jid;
		QString bare;
		Split (OurJID_, &jid, &bare);
		QString logName = jid + ".qxmpp.log";
		logName.replace ('@', '_');
		const QString& path = Util::CreateIfNotExists ("azoth/xoox/logs").filePath (logName);
		QFileInfo info (path);
		if (info.size () > 1024 * 1024 * 10)
			QFile::remove (path);

		QXmppLogger *logger = new QXmppLogger (Client_);
		logger->setLoggingType (QXmppLogger::FileLogging);
		logger->setLogFilePath (path);
		logger->setMessageTypes (QXmppLogger::AnyMessage);
		Client_->setLogger (logger);
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

	void ClientConnection::handlePendingForm (QXmppDataForm *formObj, const QString& from)
	{
		std::unique_ptr<QXmppDataForm> form (formObj);
		FormBuilder fb (from, BobManager_);

		QDialog dia;
		dia.setWindowTitle (tr ("Data form from %1").arg (from));
		dia.setLayout (new QVBoxLayout ());

		dia.layout ()->addWidget (new QLabel (tr ("You have received "
						"dataform from %1:").arg (from)));
		dia.layout ()->addWidget (fb.CreateForm (*form));
		QDialogButtonBox *box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect (box,
				SIGNAL (accepted ()),
				&dia,
				SLOT (accept ()));
		connect (box,
				SIGNAL (rejected ()),
				&dia,
				SLOT (reject ()));
		dia.layout ()->addWidget (box);
		dia.setWindowModality (Qt::WindowModal);
		if (dia.exec () != QDialog::Accepted)
			return;

		QXmppMessage msg ("", from);
		msg.setType (QXmppMessage::Normal);
		QXmppDataForm subForm = fb.GetForm ();
		subForm.setType (QXmppDataForm::Submit);
		msg.setExtensions (XooxUtil::Form2XmppElem (subForm));
		Client_->sendPacket (msg);
	}

	void ClientConnection::handleConnected ()
	{
		IsConnected_ = true;
		emit statusChanged (EntryStatus (LastState_.State_, LastState_.Status_));

		Client_->vCardManager ().requestVCard (OurBareJID_);

		connect (BMManager_,
				SIGNAL (bookmarksReceived (const QXmppBookmarkSet&)),
				this,
				SLOT (handleBookmarksReceived (const QXmppBookmarkSet&)),
				Qt::UniqueConnection);

		AnnotationsManager_->refetchNotes ();

		Q_FOREACH (RoomHandler *rh, RoomHandlers_)
			rh->Join ();
	}

	void ClientConnection::handleDisconnected ()
	{
		emit statusChanged (EntryStatus (SOffline, LastState_.Status_));
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
				str = tr ("socket error: %1.")
						.arg (Util::GetSocketErrorString (Client_->socketError ()));
			}
			break;
		case QXmppClient::KeepAliveError:
			str = tr ("keep-alive error.");
			break;
		case QXmppClient::XmppStreamError:
			str = tr ("error while connecting: ");
			str += HandleErrorCondition (Client_->xmppStreamError ());
			break;
		case QXmppClient::NoError:
			str = tr ("no error.");
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
				tr ("Account %1:").arg (OurJID_) +
					' ' + str,
				PCritical_);
		Core::Instance ().SendEntity (e);
	}

	void ClientConnection::handleIqReceived (const QXmppIq& iq)
	{
		if (iq.error ().isValid ())
			HandleError (iq);
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

		QPair<QString, PEPEventBase*> initialEvent;
		Q_FOREACH (initialEvent, InitialEventQueue_)
		{
			handlePEPEvent (initialEvent.first, initialEvent.second);
			delete initialEvent.second;
		}
		InitialEventQueue_.clear ();
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
			entry->SetStatus (XooxUtil::PresenceToStatus (pres), resource);
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

		if (jid.isEmpty ())
			jid = OurBareJID_;

		Q_FOREACH (auto f, VCardFetchCallbacks_.take (jid))
			f (vcard);

		if (JID2CLEntry_.contains (jid))
			JID2CLEntry_ [jid]->SetVCard (vcard);
		else if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->GetParticipantEntry (nick)->SetVCard (vcard);
		else if (OurBareJID_ == jid)
			SelfContact_->SetVCard (vcard);
	}

	void ClientConnection::handleVersionReceived (const QXmppVersionIq& version)
	{
		QString jid;
		QString nick;
		Split (version.from (), &jid, &nick);

		if (JID2CLEntry_.contains (jid))
			JID2CLEntry_ [jid]->SetClientVersion (nick, version);
		else if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->GetParticipantEntry (nick)->SetClientVersion (QString (), version);
		else if (OurBareJID_ == jid)
			SelfContact_->SetClientVersion (nick, version);
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

		if (jid == OurBareJID_)
		{
			if (OurJID_ == pres.from ())
				emit statusChanged (XooxUtil::PresenceToStatus (pres));

			if (pres.type () == QXmppPresence::Available)
			{
				SelfContact_->SetClientInfo (resource, pres);
				SelfContact_->UpdatePriority (resource, pres.status ().priority ());
				SelfContact_->SetStatus (XooxUtil::PresenceToStatus (pres), resource);
			}
			else
				SelfContact_->RemoveVariant (resource);

			return;
		}
		else if (!JID2CLEntry_.contains (jid))
		{
			if (ODSEntries_.contains (jid))
				ConvertFromODS (jid, Client_->rosterManager ().getRosterEntry (jid));
			else
				return;
		}

		JID2CLEntry_ [jid]->HandlePresence (pres, resource);
		if (SignedPresences_.remove (jid))
		{
			qDebug () << "got signed presence" << jid;
		}
	}

	namespace
	{
		void HandleMessageForEntry (EntryBase *entry,
				const QXmppMessage& msg, const QString& resource,
				ClientConnection *conn)
		{
			if (msg.state ())
				entry->UpdateChatState (msg.state (), resource);

			if (!msg.body ().isEmpty ())
			{
				GlooxMessage *gm = new GlooxMessage (msg, conn);
				entry->HandleMessage (gm);
			}

			if (msg.isAttentionRequested ())
				entry->HandleAttentionMessage (msg);
		}
	}

	void ClientConnection::handleMessageReceived (QXmppMessage msg)
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

		if (EncryptedMessages_.contains (msg.from ()))
			msg.setBody (EncryptedMessages_.take (msg.from ()));

		if (AwaitingRIEXItems_.contains (msg.from ()))
		{
			HandleRIEX (msg.from (), AwaitingRIEXItems_.take (msg.from ()), msg.body ());
			return;
		}
		else if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->HandleMessage (msg, resource);
		else if (JID2CLEntry_.contains (jid))
			HandleMessageForEntry (JID2CLEntry_ [jid], msg, resource, this);
		else if (!Client_->rosterManager ().isRosterReceived ())
			OfflineMsgQueue_ << msg;
		else if (jid == OurBareJID_)
		{
			Q_FOREACH (const QXmppExtendedAddress& address, msg.extendedAddresses ())
			{
				if (address.type () == "ofrom" && !address.jid ().isEmpty ())
				{
					msg.setFrom (address.jid ());
					handleMessageReceived (msg);
					return;
				}
			}
			HandleMessageForEntry (SelfContact_, msg, resource, this);
		}
		else
		{
			Q_FOREACH (const auto& extension, msg.extensions ())
				if (extension.tagName () == "x" &&
						extension.attribute ("xmlns") == "jabber:x:conference")
					return;

			qWarning () << Q_FUNC_INFO
					<< "could not find source for"
					<< msg.from ()
					<< "; creating new item";

			GlooxCLEntry *entry = new GlooxCLEntry (jid, Account_);
			JID2CLEntry_ [jid] = entry;
			emit gotRosterItems (QList<QObject*> () << entry);

			handleMessageReceived (msg);
		}
	}

	void ClientConnection::handlePEPEvent (const QString& from, PEPEventBase *event)
	{
		QString bare;
		QString resource;
		Split (from, &bare, &resource);

		if (bare == OurBareJID_)
			SelfContact_->HandlePEPEvent (resource, event);
		else if (!JID2CLEntry_.contains (bare))
		{
			if (JID2CLEntry_.isEmpty ())
				InitialEventQueue_ << qMakePair (from, event->Clone ());
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown PEP event source"
						<< from
						<< "; known entries:"
						<< JID2CLEntry_.keys ().size ();
		}
		else
			JID2CLEntry_ [bare]->HandlePEPEvent (resource, event);
	}

	void ClientConnection::handlePEPAvatarUpdated (const QString& from, const QImage& image)
	{
		QString bare;
		QString resource;
		Split (from, &bare, &resource);

		if (!JID2CLEntry_.contains (from))
			return;

		JID2CLEntry_ [from]->SetAvatar (image);
	}

	void ClientConnection::handleMessageDelivered (const QString& msgId)
	{
		QPointer<GlooxMessage> msg = UndeliveredMessages_.take (msgId);
		if (msg)
			msg->SetDelivered (true);
	}

	void ClientConnection::handleCaptchaReceived (const QString& jid, const QXmppDataForm& dataForm)
	{
		FormBuilder builder (jid, BobManager_);

		std::auto_ptr<QDialog> dialog (new QDialog ());
		QWidget *widget = builder.CreateForm (dataForm, dialog.get ());
		dialog->setWindowTitle (widget->windowTitle ().isEmpty () ?
				tr ("Enter CAPTCHA") :
				widget->windowTitle ());
		dialog->setLayout (new QVBoxLayout ());
		dialog->layout ()->addWidget (widget);
		QDialogButtonBox *box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		dialog->layout ()->addWidget (box);

		connect (box,
				SIGNAL (accepted ()),
				dialog.get (),
				SLOT (accept ()));
		connect (box,
				SIGNAL (rejected ()),
				dialog.get (),
				SLOT (reject ()));

		if (dialog->exec () != QDialog::Accepted)
			return;

		QXmppDataForm form = builder.GetForm ();
		CaptchaManager_->sendResponse (jid, form);
	}

	void ClientConnection::handleRoomInvitation (const QString& room,
			const QString& inviter, const QString& reason)
	{
		const QStringList& split = room.split ('@', QString::SkipEmptyParts);

		QVariantMap identifying;
		identifying ["HumanReadableName"] = QString ("%2 (%1)")
				.arg (Account_->GetOurNick ())
				.arg (room);
		identifying ["AccountID"] = Account_->GetAccountID ();
		identifying ["Nick"] = Account_->GetOurNick ();
		identifying ["Room"] = split.value (0);
		identifying ["Server"] = split.value (1);

		emit gotMUCInvitation (identifying, inviter, reason);
	}

	void ClientConnection::handleGotRIEXItems (QString msgFrom, QList<RIEXManager::Item> items, bool msgPending)
	{
		if (msgPending)
			AwaitingRIEXItems_ [msgFrom] += items;
		else
			HandleRIEX (msgFrom, items);
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
			QTimer::singleShot (3000,
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

		const JoinQueueItem& it = JoinQueue_.takeFirst ();
		emit gotRosterItems (QList<QObject*> () << JoinRoom (it.RoomJID_, it.Nickname_));

		if (!JoinQueue_.isEmpty ())
			QTimer::singleShot (800,
					this,
					SLOT (handleAutojoinQueue ()));
	}

	void ClientConnection::handleDiscoInfo (const QXmppDiscoveryIq& iq)
	{
		const QString& jid = iq.from ();
		if (AwaitingDiscoInfo_.contains (jid))
			AwaitingDiscoInfo_.take (jid) (iq);
	}

	void ClientConnection::handleDiscoItems (const QXmppDiscoveryIq& iq)
	{
		const QString& jid = iq.from ();
		if (AwaitingDiscoItems_.contains (jid))
			AwaitingDiscoItems_.take (jid) (iq);
	}

	void ClientConnection::handleEncryptedMessageReceived (const QString& id,
			const QString& decrypted)
	{
		EncryptedMessages_ [id] = decrypted;
	}

	void ClientConnection::handleSignedMessageReceived (const QString&)
	{
	}

	void ClientConnection::handleSignedPresenceReceived (const QString&)
	{
	}

	void ClientConnection::handleInvalidSignatureReceived (const QString& id)
	{
		qDebug () << Q_FUNC_INFO << id;
	}

	void ClientConnection::handleLog (QXmppLogger::MessageType type, const QString& msg)
	{
		QString entryId;
		QDomDocument doc;
		if (doc.setContent (msg))
		{
			const auto& elem = doc.documentElement ();
			if (type == QXmppLogger::ReceivedMessage)
				entryId = elem.attribute ("from");
			else if (type == QXmppLogger::SentMessage)
				entryId = elem.attribute ("to");
		}
		switch (type)
		{
		case QXmppLogger::SentMessage:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PDOut, entryId);
			break;
		case QXmppLogger::ReceivedMessage:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PDIn, entryId);
			break;
		default:
			break;
		}
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
			if (!JID2CLEntry_.contains (jid))
			{
				GlooxCLEntry *entry = new GlooxCLEntry (jid, Account_);
				JID2CLEntry_ [jid] = entry;
				emit gotRosterItems (QList<QObject*> () << entry);
			}
			JID2CLEntry_ [jid]->SetAuthRequested (true);
			emit gotSubscriptionRequest (JID2CLEntry_ [jid], QString ());
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

	void ClientConnection::HandleRIEX (QString msgFrom, QList<RIEXManager::Item> origItems, QString body)
	{
		QList<RIEXItem> items;
		Q_FOREACH (const RIEXManager::Item& item, origItems)
		{
			RIEXItem ri =
			{
				static_cast<RIEXItem::Action> (item.GetAction ()),
				item.GetJID (),
				item.GetName (),
				item.GetGroups ()
			};

			items << ri;
		}

		QString jid;
		QString resource;
		Split (msgFrom, &jid, &resource);

		if (!items.isEmpty ())
			QMetaObject::invokeMethod (Account_,
					"riexItemsSuggested",
					Q_ARG (QList<LeechCraft::Azoth::RIEXItem>, items),
					Q_ARG (QObject*, JID2CLEntry_.value (jid)),
					Q_ARG (QString, body));
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

	void ClientConnection::InitializeQCA ()
	{
#ifdef ENABLE_CRYPT
		PGPManager_ = new PgpManager ();

		Client_->addExtension (PGPManager_);
		connect (PGPManager_,
				SIGNAL (encryptedMessageReceived (QString, QString)),
				this,
				SLOT (handleEncryptedMessageReceived (QString, QString)));
		connect (PGPManager_,
				SIGNAL (signedMessageReceived (const QString&)),
				this,
				SLOT (handleSignedMessageReceived (const QString&)));
		connect (PGPManager_,
				SIGNAL (signedPresenceReceived (const QString&)),
				this,
				SLOT (handleSignedPresenceReceived (const QString&)));
		connect (PGPManager_,
				SIGNAL (invalidSignatureReceived (const QString&)),
				this,
				SLOT (handleInvalidSignatureReceived (const QString&)));
#endif
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
				FetchVersion (bareJID);
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
		return entry;
	}
}
}
}
