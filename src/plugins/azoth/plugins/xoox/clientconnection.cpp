/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clientconnection.h"
#include <QTimer>
#include <QDir>
#include <QMimeDatabase>
#include <QtDebug>
#include <QXmppClient.h>
#include <QXmppMucManager.h>
#include <QXmppVersionManager.h>
#include <QXmppRosterManager.h>
#include <QXmppVCardManager.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppPubSubItem.h>
#include <QXmppPubSubIq.h>
#include <util/sll/containerconversions.h>
#include <util/sll/prelude.h>
#include <util/xpc/util.h>
#include <util/network/socketerrorstrings.h>
#include <util/sys/sysinfo.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
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
#include "xeps/pubsubmanager.h"
#include "useractivity.h"
#include "usermood.h"
#include "usertune.h"
#include "userlocation.h"
#include "xeps/privacylistsmanager.h"
#include "util.h"
#include "selfcontact.h"
#include "xeps/adhoccommandserver.h"
#include "useravatarmanager.h"
#include "sdmanager.h"
#include "xep0232handler.h"
#include "pepmicroblog.h"
#include "clientconnectionerrormgr.h"
#include "accountsettingsholder.h"
#include "crypthandler.h"
#include "serverinfostorage.h"
#include "xmlsettingsmanager.h"
#include "inforequestpolicymanager.h"
#include "xeps/xep0313manager.h"
#include "xeps/carbonsmanager.h"
#include "xeps/pingmanager.h"
#include "xep0334utils.h"
#include "sslerrorshandler.h"
#include "clientconnectionextensionsmanager.h"
#include "xeps/riexmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	ClientConnection::ClientConnection (GlooxAccount *account)
	: Account_ (account)
	, Settings_ (account->GetSettings ())
	, Client_ (new QXmppClient (this))
	, MUCManager_ (new QXmppMucManager)
	, DiscoveryManager_ (Client_->findExtension<QXmppDiscoveryManager> ())
	, PubSubManager_ (new PubSubManager)
	, PrivacyListsManager_ (new PrivacyListsManager (this))
	, AnnotationsManager_ (0)
	, UserAvatarManager_ (0)
	, SDManager_ (new SDManager (this))
	, Xep0313Manager_ (new Xep0313Manager (*this))
	, CarbonsManager_ (new CarbonsManager)
	, CryptHandler_ (new CryptHandler (this))
	, ErrorMgr_ (new ClientConnectionErrorMgr (this))
	, InfoReqPolicyMgr_ (new InfoRequestPolicyManager (this))
	, DiscoManagerWrapper_ (new DiscoManagerWrapper (DiscoveryManager_, this))
	, ExtsMgr_ (std::make_unique<ClientConnectionExtensionsManager> (*this, *Client_))
	, OurJID_ (Settings_->GetFullJID ())
	, SelfContact_ (new SelfContact (OurJID_, account))
	, CapsManager_ (new CapsManager (DiscoveryManager_, this, account->GetParentProtocol ()->GetCapsDatabase ()))
	, ServerInfoStorage_ (new ServerInfoStorage (this, Settings_))
	, VCardQueue_ (new FetchQueue ([this] (QString str, bool report)
				{
					const auto& id = Exts ().Get<QXmppVCardManager> ().requestVCard (str);
					ErrorMgr_->Whitelist (id, report);
				},
				OurJID_.contains ("gmail.com") ? 3000 : 1500, 1, this))
	, CapsQueue_ (new FetchQueue ([this] (QString str, bool report)
				{
					const auto& id = DiscoveryManager_->requestInfo (str, "");
					ErrorMgr_->Whitelist (id, report);
				},
				OurJID_.contains ("gmail.com") ? 1000 : 400, 1, this))
	, VersionQueue_ (new FetchQueue ([this] (QString str, bool report)
				{
					const auto& id = Exts ().Get<QXmppVersionManager> ().requestVersion (str);
					ErrorMgr_->Whitelist (id, report);
				},
				OurJID_.contains ("gmail.com") ? 2000 : 1000, 1, this))
	{
		SetOurJID (OurJID_);

		connect (ErrorMgr_,
				SIGNAL (serverAuthFailed ()),
				this,
				SIGNAL (serverAuthFailed ()));

		handlePriorityChanged (Settings_->GetPriority ());

		const auto proxy = account->GetParentProtocol ()->GetProxyObject ();

		PubSubManager_->RegisterCreator<UserActivity> ();
		PubSubManager_->RegisterCreator<UserMood> ();
		PubSubManager_->RegisterCreator<UserTune> ();
		PubSubManager_->RegisterCreator<UserLocation> ();
		PubSubManager_->RegisterCreator<PEPMicroblog> ();
		PubSubManager_->SetAutosubscribe<UserActivity> (true);
		PubSubManager_->SetAutosubscribe<UserMood> (true);
		PubSubManager_->SetAutosubscribe<UserTune> (true);
		PubSubManager_->SetAutosubscribe<UserLocation> (true);
		PubSubManager_->SetAutosubscribe<PEPMicroblog> (true);

		connect (PubSubManager_,
				SIGNAL (gotEvent (const QString&, PEPEventBase*)),
				this,
				SLOT (handlePEPEvent (const QString&, PEPEventBase*)));

		UserAvatarManager_ = new UserAvatarManager (proxy->GetAvatarsManager (), this);
		connect (UserAvatarManager_,
				SIGNAL (avatarUpdated (QString)),
				this,
				SLOT (handlePEPAvatarUpdated (QString)));

		CryptHandler_->Init ();

		Client_->addExtension (PubSubManager_);
		Client_->addExtension (MUCManager_);
		Client_->addExtension (PrivacyListsManager_);
		Client_->addExtension (new AdHocCommandServer (this, proxy));
		Client_->addExtension (Xep0313Manager_);
		Client_->addExtension (CarbonsManager_);

		connect (CarbonsManager_,
				SIGNAL (gotMessage (QXmppMessage)),
				this,
				SLOT (handleCarbonsMessage (QXmppMessage)));

		AnnotationsManager_ = new AnnotationsManager (*this, this);

		DiscoveryManager_->setClientCapabilitiesNode ("http://leechcraft.org/azoth");

		auto& vm = Exts ().Get<QXmppVersionManager> ();
		vm.setClientName ("LeechCraft Azoth");
		handleVersionSettingsChanged ();
		XmlSettingsManager::Instance ().RegisterObject ("AdvertiseQtVersion",
				this, "handleVersionSettingsChanged");
		XmlSettingsManager::Instance ().RegisterObject ("AdvertiseOSVersion",
				this, "handleVersionSettingsChanged");

		connect (Client_,
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));
		connect (Client_,
				SIGNAL (disconnected ()),
				this,
				SLOT (handleDisconnected ()));
		connect (Client_,
				SIGNAL (iqReceived (QXmppIq)),
				this,
				SLOT (handleIqReceived (QXmppIq)));
		connect (Client_,
				SIGNAL (presenceReceived (QXmppPresence)),
				this,
				SLOT (handlePresenceChanged (QXmppPresence)));
		connect (Client_,
				SIGNAL (messageReceived (QXmppMessage)),
				this,
				SLOT (handleMessageReceived (QXmppMessage)));

		connect (MUCManager_,
				SIGNAL (invitationReceived (QString, QString, QString)),
				this,
				SLOT (handleRoomInvitation (QString, QString, QString)));

		auto& rosterManager = Exts ().Get<QXmppRosterManager> ();
		connect (&rosterManager,
				SIGNAL (rosterReceived ()),
				this,
				SLOT (handleRosterReceived ()));
		connect (&rosterManager,
				SIGNAL (itemAdded (QString)),
				this,
				SLOT (handleRosterChanged (QString)));
		connect (&rosterManager,
				SIGNAL (itemChanged (QString)),
				this,
				SLOT (handleRosterChanged (QString)));
		connect (&rosterManager,
				SIGNAL (itemRemoved (QString)),
				this,
				SLOT (handleRosterItemRemoved (QString)));

		connect (&Exts ().Get<QXmppVCardManager> (),
				SIGNAL (vCardReceived (QXmppVCardIq)),
				this,
				SLOT (handleVCardReceived (QXmppVCardIq)));

		connect (&Exts ().Get<QXmppVersionManager> (),
				SIGNAL (versionReceived (QXmppVersionIq)),
				this,
				SLOT (handleVersionReceived (QXmppVersionIq)));

		connect (Settings_,
				SIGNAL (kaParamsChanged (QPair<int, int>)),
				this,
				SLOT (setKAParams (QPair<int, int>)));
		connect (Settings_,
				SIGNAL (photoHashChanged (QByteArray)),
				this,
				SLOT (handlePhotoHash ()));
		connect (Settings_,
				SIGNAL (priorityChanged (int)),
				this,
				SLOT (handlePriorityChanged (int)));

		connect (Settings_,
				SIGNAL (messageCarbonsSettingsChanged ()),
				this,
				SLOT (handleMessageCarbonsSettingsChanged ()));

		const auto sslHandler = new SslErrorsHandler { Client_ };
		connect (sslHandler,
				SIGNAL (sslErrors (QList<QSslError>, ICanHaveSslErrors::ISslErrorsReaction_ptr)),
				this,
				SIGNAL (sslErrors (QList<QSslError>, ICanHaveSslErrors::ISslErrorsReaction_ptr)));
	}

	ClientConnection::~ClientConnection ()
	{
		qDeleteAll (RoomHandlers_);
	}

	void ClientConnection::SetState (const GlooxAccountState& state)
	{
		LastState_ = state;

		auto pres = XooxUtil::StatusToPresence (state.State_, state.Status_, state.Priority_);
		if (!Settings_->GetPhotoHash ().isEmpty ())
		{
			pres.setVCardUpdateType (QXmppPresence::VCardUpdateValidPhoto);
			pres.setPhotoHash (Settings_->GetPhotoHash ());
		}

		if (IsConnected_ ||
				state.State_ == SOffline)
			Client_->setClientPresence (pres);

		for (auto rh : RoomHandlers_)
			rh->SetPresence (pres);

		if (!IsConnected_ &&
				state.State_ != SOffline)
		{
			emit statusChanged (EntryStatus (SConnecting, QString ()));
			if (FirstTimeConnect_)
				emit needPassword ();

			QXmppConfiguration conf;
			conf.setJid (OurJID_);
			conf.setPassword (Password_);
			const QString& host = Settings_->GetHost ();
			const int port = Settings_->GetPort ();
			if (!host.isEmpty ())
				conf.setHost (host);
			if (port >= 0)
				conf.setPort (port);
			conf.setKeepAliveInterval (Settings_->GetKAParams ().first);
			conf.setKeepAliveTimeout (Settings_->GetKAParams ().second);
			conf.setStreamSecurityMode (Settings_->GetTLSMode ());
			Client_->connectToServer (conf, pres);

			FirstTimeConnect_ = false;
		}

		if (state.State_ == SOffline)
		{
			VCardQueue_->Clear ();
			CapsQueue_->Clear ();
			VersionQueue_->Clear ();

			for (const auto& jid : JID2CLEntry_.keys ())
			{
				auto entry = JID2CLEntry_.take (jid);
				ODSEntries_ [jid] = entry;
				entry->Convert2ODS ();
			}
			SelfContact_->RemoveVariant (OurResource_, true);

			emit statusChanged (EntryStatus (SOffline, state.Status_));
			Client_->disconnectFromServer ();
			IsConnected_ = false;
		}
	}

	GlooxAccountState ClientConnection::GetLastState () const
	{
		return LastState_;
	}

	bool ClientConnection::IsConnected () const
	{
		return IsConnected_;
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

		std::tie (OurBareJID_, OurResource_) = Split (jid);

		SelfContact_->UpdateJID (jid);
	}

	QString ClientConnection::GetOurResource () const
	{
		return OurResource_;
	}

	RoomCLEntry* ClientConnection::JoinRoom (const QString& jid, const QString& nick, bool asAutojoin)
	{
		if (RoomHandlers_.contains (jid))
		{
			if (!asAutojoin)
				GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Azoth",
						tr ("This room is already joined."),
						Priority::Critical));
			return nullptr;
		}

		const auto rh = new RoomHandler (jid, nick, asAutojoin, Account_);
		RoomHandlers_ [jid] = rh;
		return rh->GetCLEntry ();
	}

	void ClientConnection::Unregister (RoomHandler *rh)
	{
		RoomHandlers_.remove (rh->GetRoomJID ());
	}

	void ClientConnection::CreateEntry (const QString& jid)
	{
		GlooxCLEntry *entry = new GlooxCLEntry (jid, Account_);
		JID2CLEntry_ [jid] = entry;
		emit gotRosterItems ({ entry });
	}

	ClientConnectionExtensionsManager& ClientConnection::Exts () const
	{
		return *ExtsMgr_;
	}

	DiscoManagerWrapper* ClientConnection::GetDiscoManagerWrapper () const
	{
		return DiscoManagerWrapper_;
	}

	QXmppMucManager* ClientConnection::GetMUCManager () const
	{
		return MUCManager_;
	}

	QXmppDiscoveryManager* ClientConnection::GetQXmppDiscoveryManager () const
	{
		return DiscoveryManager_;
	}

	QXmppVersionManager* ClientConnection::GetVersionManager () const
	{
		return &Exts ().Get<QXmppVersionManager> ();
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

	UserAvatarManager* ClientConnection::GetUserAvatarManager () const
	{
		return UserAvatarManager_;
	}

	SDManager* ClientConnection::GetSDManager () const
	{
		return SDManager_;
	}

	Xep0313Manager* ClientConnection::GetXep0313Manager () const
	{
		return Xep0313Manager_;
	}

	InfoRequestPolicyManager* ClientConnection::GetInfoReqPolicyManager () const
	{
		return InfoReqPolicyMgr_;
	}

	CryptHandler* ClientConnection::GetCryptHandler () const
	{
		return CryptHandler_;
	}

	ServerInfoStorage* ClientConnection::GetServerInfoStorage () const
	{
		return ServerInfoStorage_;
	}

	void ClientConnection::RequestInfo (const QString& jid) const
	{
		if (JID2CLEntry_.contains (jid))
			for (const auto& variant : JID2CLEntry_ [jid]->Variants ())
				CapsQueue_->Schedule (jid + '/' + variant, FetchQueue::PHigh);
		else
			CapsQueue_->Schedule (jid, FetchQueue::PLow);
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
		iq.setItems ({ item });
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
		Exts ().Get<QXmppRosterManager> ().addItem (id, name, Util::AsSet (groups));
	}

	void ClientConnection::Subscribe (const QString& id,
			const QString& msg, const QString& name, const QStringList& groups)
	{
		qDebug () << "Subscribe" << id;
		auto& rm = Exts ().Get<QXmppRosterManager> ();
		if (!rm.getRosterBareJids ().contains (id))
			rm.addItem (id, name, Util::AsSet (groups));
		rm.subscribe (id, msg);
		rm.acceptSubscription (id, msg);
	}

	void ClientConnection::Unsubscribe (const QString& jid, const QString& reason)
	{
		qDebug () << "Unsubscribe" << jid;
		Exts ().Get<QXmppRosterManager> ().unsubscribe (jid, reason);
	}

	void ClientConnection::GrantSubscription (const QString& jid, const QString& reason)
	{
		qDebug () << "GrantSubscription" << jid;
		Exts ().Get<QXmppRosterManager> ().acceptSubscription (jid, reason);
		if (JID2CLEntry_ [jid])
			JID2CLEntry_ [jid]->SetAuthRequested (false);
	}

	void ClientConnection::RevokeSubscription (const QString& jid, const QString& reason)
	{
		qDebug () << "RevokeSubscription" << jid;
		Exts ().Get<QXmppRosterManager> ().refuseSubscription (jid, reason);
		if (JID2CLEntry_ [jid])
			JID2CLEntry_ [jid]->SetAuthRequested (false);
	}

	void ClientConnection::Remove (GlooxCLEntry *entry)
	{
		const QString& jid = entry->GetJID ();

		auto& rm = Exts ().Get<QXmppRosterManager> ();
		if (rm.getRosterBareJids ().contains (jid))
			rm.removeItem (jid);
		else
		{
			qWarning () << Q_FUNC_INFO
					<< jid
					<< "isn't present in roster manager, removing directly";
			handleRosterItemRemoved (jid);
		}

		if (ODSEntries_.contains (jid))
		{
			const auto otherEntry = ODSEntries_.take (jid);
			if (otherEntry != entry)
				qWarning () << Q_FUNC_INFO
						<< "stored ODS entry isn't equal to entry for"
						<< jid
						<< "!";
			emit rosterItemRemoved (otherEntry);

			delete otherEntry;
		}
	}

	ClientConnectionErrorMgr* ClientConnection::GetErrorManager () const
	{
		return ErrorMgr_;
	}

	void ClientConnection::SendPacketWCallback (const QXmppIq& packet, PacketCallback_t cb)
	{
		AwaitingPacketCallbacks_ [packet.id ()] = cb;
		Client_->sendPacket (packet);
	}

	void ClientConnection::AddCallback (const QString& id, const PacketCallback_t& cb)
	{
		AwaitingPacketCallbacks_ [id] = cb;
	}

	void ClientConnection::SendMessage (GlooxMessage *msgObj)
	{
		auto msg = msgObj->GetNativeMessage ();
		CryptHandler_->ProcessOutgoing (msg, msgObj);

		if (msgObj->IsOTRMessage ())
		{
			CarbonsManager_->ExcludeMessage (msg);
			Xep0334::SetHint (msg, Xep0334::MessageHint::NoCopies);
			Xep0334::SetHint (msg, Xep0334::MessageHint::NoPermStorage);
			Xep0334::SetHint (msg, Xep0334::MessageHint::NoStorage);
		}

		Client_->sendPacket (msg);
	}

	GlooxAccount* ClientConnection::GetAccount () const
	{
		return Account_;
	}

	QXmppClient* ClientConnection::GetClient () const
	{
		return Client_;
	}

	QObject* ClientConnection::GetCLEntry (const QString& fullJid) const
	{
		auto [bare, variant] = Split (fullJid);
		return GetCLEntry (bare, variant);
	}

	QObject* ClientConnection::GetCLEntry (const QString& bareJid, const QString& variant) const
	{
		if (const auto rh = RoomHandlers_.value (bareJid))
		{
			if (variant.isEmpty ())
				return rh->GetCLEntry ();
			else
				return rh->GetParticipantEntry (variant).get ();
		}
		else if (bareJid == OurBareJID_)
			return SelfContact_;
		else if (const auto entry = JID2CLEntry_.value (bareJid))
			return entry;
		else if (const auto entry = ODSEntries_.value (bareJid))
			return entry;
		else
		{
			auto [trueBare, trueVar] = Split (bareJid);
			if (trueBare != bareJid)
				return GetCLEntry (trueBare, trueVar);

			return nullptr;
		}
	}

	GlooxCLEntry* ClientConnection::AddODSCLEntry (OfflineDataSource_ptr ods)
	{
		GlooxCLEntry *entry = new GlooxCLEntry (ods, Account_);
		ODSEntries_ [entry->GetJID ()] = entry;

		emit gotRosterItems ({ entry });

		return entry;
	}

	QList<QObject*> ClientConnection::GetCLEntries () const
	{
		QList<QObject*> result { SelfContact_ };

		const auto totalRoomParticipants = std::accumulate (RoomHandlers_.begin (), RoomHandlers_.end (), 0,
				[] (int acc, RoomHandler *rh) { return acc + rh->GetParticipants ().size (); });
		result.reserve (1 + JID2CLEntry_.size () + ODSEntries_.size () + totalRoomParticipants + RoomHandlers_.size ());

		std::copy (JID2CLEntry_.begin (), JID2CLEntry_.end (), std::back_inserter (result));
		std::copy (ODSEntries_.begin (), ODSEntries_.end (), std::back_inserter (result));

		for (const auto rh : RoomHandlers_)
		{
			result << rh->GetCLEntry ();
			result += rh->GetParticipants ();
		}

		return result;
	}

	void ClientConnection::FetchVCard (const QString& jid, bool reportErrors)
	{
		ScheduleFetchVCard (jid, reportErrors);
	}

	void ClientConnection::FetchVCard (const QString& jid, VCardCallback_t callback, bool reportErrors)
	{
		VCardFetchCallbacks_ [jid] << callback;
		ScheduleFetchVCard (jid, reportErrors);
	}

	void ClientConnection::FetchVersion (const QString& jid, bool reportErrors)
	{
		VersionQueue_->Schedule (jid, FetchQueue::Priority::PLow, reportErrors);
	}

	GlooxMessage* ClientConnection::CreateMessage (IMessage::Type type,
			const QString& resource, const QString& body, const QString& jid)
	{
		GlooxMessage *msg = new GlooxMessage (type,
				IMessage::Direction::Out,
				jid,
				resource,
				this);
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());
		return msg;
	}

	ClientConnection::SplitResult ClientConnection::Split (const QString& jid)
	{
		const int pos = jid.indexOf ('/');
		return
		{
			.Bare_ = jid.left (pos),
			.Resource_ = pos >= 0 ? jid.mid (pos + 1) : QString {}
		};
	}

	void ClientConnection::handleConnected ()
	{
		IsConnected_ = true;
		emit connected ();
		emit statusChanged ({ LastState_.State_, LastState_.Status_ });

		Exts ().Get<QXmppVCardManager> ().requestVCard (OurBareJID_);

		for (auto rh : RoomHandlers_)
			rh->Join ();

		PrivacyListsManager_->QueryLists ();

		handleMessageCarbonsSettingsChanged ();
	}

	void ClientConnection::handleDisconnected ()
	{
		IsConnected_ = false;
		emit statusChanged (EntryStatus (SOffline, LastState_.Status_));
	}

	void ClientConnection::handleIqReceived (const QXmppIq& iq)
	{
		ErrorMgr_->HandleIq (iq);
		InvokeCallbacks (iq);
	}

	void ClientConnection::handleRosterReceived ()
	{
		const auto& rm = Exts ().Get<QXmppRosterManager> ();
		QObjectList items;
		for (const auto& bareJid : rm.getRosterBareJids ())
		{
			const auto& re = rm.getRosterEntry (bareJid);
			const auto entry = CreateCLEntry (re);
			items << entry;
			const auto& presences = rm.getAllPresencesForBareJid (re.bareJid ());
			for (const auto& resource : presences.keys ())
				entry->SetClientInfo (resource, presences [resource]);
		}
		emit gotRosterItems (items);

		for (const auto& msg : OfflineMsgQueue_)
			handleMessageReceived (msg);
		OfflineMsgQueue_.clear ();

		for (const auto& initialEvent : InitialEventQueue_)
		{
			handlePEPEvent (initialEvent.first, initialEvent.second);
			delete initialEvent.second;
		}
		InitialEventQueue_.clear ();
	}

	void ClientConnection::handleRosterChanged (const QString& bareJid)
	{
		const auto& rm = Exts ().Get<QXmppRosterManager> ();
		const auto& presences = rm.getAllPresencesForBareJid (bareJid);

		if (!JID2CLEntry_.contains (bareJid))
			emit gotRosterItems ({ CreateCLEntry (bareJid) });

		const auto entry = JID2CLEntry_ [bareJid];
		for (const auto& resource : presences.keys ())
		{
			const auto& pres = presences [resource];
			entry->SetClientInfo (resource, pres);
			entry->SetStatus (XooxUtil::PresenceToStatus (pres), resource, pres);
		}
		entry->UpdateRI (rm.getRosterEntry (bareJid));
	}

	void ClientConnection::handleRosterItemRemoved (const QString& bareJid)
	{
		qDebug () << "RosterItemRemoved" << bareJid;
		if (!JID2CLEntry_.contains (bareJid))
			return;

		const auto entry = JID2CLEntry_.take (bareJid);
		emit rosterItemRemoved (entry);
		entry->deleteLater ();
	}

	void ClientConnection::handleVCardReceived (const QXmppVCardIq& vcard)
	{
		auto [jid, nick] = Split (vcard.from ());

		if (jid.isEmpty ())
			jid = OurBareJID_;

		for (const auto& f : VCardFetchCallbacks_.take (jid))
			f (vcard);

		for (const auto& f : VCardFetchCallbacks_.take (vcard.from ()))
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
		auto [jid, nick] = Split (version.from ());

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

		auto [jid, resource] = Split (pres.from ());

		if (jid == OurBareJID_)
		{
			const bool thisInstance = OurResource_ == resource;
			if (thisInstance)
				emit statusChanged (XooxUtil::PresenceToStatus (pres));

			SelfContact_->HandlePresence (pres, resource);
			return;
		}
		else if (!JID2CLEntry_.contains (jid))
		{
			if (ODSEntries_.contains (jid))
				ConvertFromODS (jid, Exts ().Get<QXmppRosterManager> ().getRosterEntry (jid));
			else
				return;
		}

		JID2CLEntry_ [jid]->HandlePresence (pres, resource);

		CryptHandler_->HandlePresence (pres, jid, resource);
	}

	namespace
	{
		void HandleMessageForEntry (EntryBase *entry,
				const QXmppMessage& msg, const QString& resource,
				ClientConnection *conn,
				bool forwarded)
		{
			if (msg.state ())
				entry->UpdateChatState (msg.state (), resource);

			if (!msg.body ().isEmpty ())
			{
				auto gm = new GlooxMessage (msg, conn);
				gm->ToggleForwarded (forwarded);
				entry->HandleMessage (gm);
			}

			if (msg.isAttentionRequested ())
				entry->HandleAttentionMessage (msg);
		}
	}

	void ClientConnection::handleMessageReceived (QXmppMessage msg, bool forwarded)
	{
		if (msg.type () == QXmppMessage::Error)
		{
			qDebug () << Q_FUNC_INFO
					<< "got error message from"
					<< msg.from ();
			ErrorMgr_->HandleMessage (msg);
			return;
		}

		auto [jid, resource] = Split (msg.from ());

		CryptHandler_->ProcessIncoming (msg);

		if (CarbonsManager_->CheckMessage (msg))
			return;
		else if (RoomHandlers_.contains (jid))
			RoomHandlers_ [jid]->HandleMessage (msg, resource);
		else if (JID2CLEntry_.contains (jid))
			HandleMessageForEntry (JID2CLEntry_ [jid], msg, resource, this, forwarded);
		else if (!Exts ().Get<QXmppRosterManager> ().isRosterReceived ())
			OfflineMsgQueue_ << msg;
		else if (jid == OurBareJID_)
		{
			for (const auto& address : msg.extendedAddresses ())
			{
				if (address.type () == "ofrom" && !address.jid ().isEmpty ())
				{
					msg.setFrom (address.jid ());
					handleMessageReceived (msg, true);
					return;
				}
			}
			HandleMessageForEntry (SelfContact_, msg, resource, this, forwarded);
		}
		else if (msg.mucInvitationJid ().isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "could not find source for"
					<< msg.from ()
					<< "; creating new item";

			CreateEntry (jid);
			handleMessageReceived (msg);
		}
	}

	void ClientConnection::handleCarbonsMessage (const QXmppMessage& msg)
	{
		if (msg.from () == OurJID_ || msg.to () == OurJID_)
			return;

		auto [jid, resource] = Split (msg.from ());

		if (jid != OurBareJID_)
		{
			handleMessageReceived (msg, true);
			return;
		}

		if (msg.body ().isEmpty ())
			return;

		std::tie (jid, resource) = Split (msg.to ());
		if (!JID2CLEntry_.contains (jid))
			return;

		auto gm = new GlooxMessage (IMessage::Type::ChatMessage, IMessage::Direction::Out,
				jid, resource, this);
		gm->SetBody (msg.body ());
		gm->SetRichBody (msg.xhtml ());
		gm->SetDateTime (msg.stamp ().isValid () ? msg.stamp () : QDateTime::currentDateTime ());

		JID2CLEntry_ [jid]->HandleMessage (gm);
	}

	void ClientConnection::handlePEPEvent (const QString& from, PEPEventBase *event)
	{
		auto [bare, resource] = Split (from);

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
						<< event->Node ()
						<< "; known entries:"
						<< JID2CLEntry_.size ();
		}
		else
			JID2CLEntry_ [bare]->HandlePEPEvent (resource, event);
	}

	void ClientConnection::handlePEPAvatarUpdated (const QString& from)
	{
		auto [bare, resource] = Split (from);

		if (bare == OurBareJID_)
		{
			SelfContact_->avatarChanged (SelfContact_);
			return;
		}

		if (!JID2CLEntry_.contains (from))
			return;

		const auto entry = JID2CLEntry_ [from];
		entry->avatarChanged (entry);
	}

	void ClientConnection::handleRoomInvitation (const QString& room,
			const QString& inviter, const QString& reason)
	{
		const auto& split = room.split ('@', Qt::SkipEmptyParts);

		QVariantMap identifying;
		identifying ["HumanReadableName"] = QString ("%2 (%1)")
				.arg (Account_->GetOurNick ())
				.arg (room);
		identifying ["AccountID"] = Account_->GetAccountID ();
		identifying ["Nick"] = Account_->GetOurNick ();
		identifying ["Room"] = split.value (0);
		identifying ["Server"] = split.value (1);

		emit Account_->mucInvitationReceived (identifying, inviter, reason);
	}

	void ClientConnection::HandleOtherPresence (const QXmppPresence& pres)
	{
		const QString& jid = pres.from ();
		switch (pres.type ())
		{
		case QXmppPresence::Subscribe:
			if (!JID2CLEntry_.contains (jid))
			{
				GlooxCLEntry *entry = new GlooxCLEntry (jid, Account_);
				JID2CLEntry_ [jid] = entry;
				emit gotRosterItems ({ entry });
			}
			JID2CLEntry_ [jid]->SetAuthRequested (true);
			emit Account_->authorizationRequested (JID2CLEntry_ [jid], pres.statusText ());
			break;
		case QXmppPresence::Subscribed:
			if (JID2CLEntry_.contains (jid))
				emit Account_->itemGrantedSubscription (JID2CLEntry_ [jid], QString ());
			break;
		case QXmppPresence::Unsubscribe:
			if (JID2CLEntry_.contains (jid))
				emit Account_->itemUnsubscribed (JID2CLEntry_ [jid], pres.statusText ());
			else
				emit Account_->itemUnsubscribed (jid, pres.statusText ());
			break;
		case QXmppPresence::Unsubscribed:
			if (JID2CLEntry_.contains (jid))
				emit Account_->itemCancelledSubscription (JID2CLEntry_ [jid], pres.statusText ());
			break;
		case QXmppPresence::Error:
		{
			auto [bare, resource] = ClientConnection::Split (jid);
			if (RoomHandlers_.contains (bare))
				RoomHandlers_ [bare]->HandleErrorPresence (pres, resource);
			else if (const auto entry = JID2CLEntry_.value (bare))
			{
				qDebug () << Q_FUNC_INFO
						<< "got error presence for"
						<< jid
						<< pres.error ().type ()
						<< pres.error ().condition ()
						<< pres.error ().text ();
				entry->SetErrorPresence (resource, pres);
			}
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

	void ClientConnection::InvokeCallbacks (const QXmppIq& iq)
	{
		if (!AwaitingPacketCallbacks_.contains (iq.id ()))
			return;

		const auto& cb = AwaitingPacketCallbacks_.take (iq.id ());
		cb (iq);
	}

	void ClientConnection::setKAParams (const QPair<int, int>& p)
	{
		if (!Client_)
			return;

		Client_->configuration ().setKeepAliveInterval (p.first);
		Client_->configuration ().setKeepAliveTimeout (p.second);
	}

	void ClientConnection::handlePhotoHash ()
	{
		if (LastState_.State_ != SOffline)
			SetState (LastState_);
	}

	void ClientConnection::handlePriorityChanged (int prio)
	{
		LastState_.Priority_ = prio;
		if (LastState_.State_ != SOffline)
			SetState (LastState_);
	}

	void ClientConnection::handleMessageCarbonsSettingsChanged ()
	{
		CarbonsManager_->SetEnabled (Settings_->IsMessageCarbonsEnabled ());
	}

	void ClientConnection::handleVersionSettingsChanged ()
	{
		const bool advertiseQt = XmlSettingsManager::Instance ()
				.property ("AdvertiseQtVersion").toBool ();
		const bool advertiseOS = XmlSettingsManager::Instance ()
				.property ("AdvertiseOSVersion").toBool ();

		const auto& sysInfo = Util::SysInfo::GetOSInfo ();
		auto infoStr = sysInfo.Name_;
		if (advertiseOS)
			infoStr += " " + sysInfo.Version_;

		auto versionStr = Core::Instance ().GetProxy ()->GetVersion ();
		if (advertiseQt)
		{
			versionStr += " (compiled with Qt ";
			versionStr += QT_VERSION_STR;
			versionStr += "; running with Qt ";
			versionStr += qVersion ();
			versionStr += ")";
		}

		auto& vm = Exts ().Get<QXmppVersionManager> ();
		vm.setClientOs (infoStr);
		vm.setClientVersion (versionStr);

		XEP0232Handler::SoftwareInformation si
		{
			{ 64, 64 },
			QUrl { "https://leechcraft.org/leechcraft.png" },
			QByteArray {},
			QMimeDatabase {}.mimeTypeForName ("image/png"),
			sysInfo.Name_,
			advertiseOS ? sysInfo.Version_ : QString (),
			vm.clientName (),
			vm.clientVersion ()
		};
		DiscoveryManager_->setClientInfoForm (XEP0232Handler::ToDataForm (si));
	}

	void ClientConnection::ScheduleFetchVCard (const QString& jid, bool report)
	{
		FetchQueue::Priority prio = !JID2CLEntry_.contains (jid) ||
					JID2CLEntry_ [jid]->GetStatus (QString ()).State_ == SOffline ?
				FetchQueue::PLow :
				FetchQueue::PHigh;
		VCardQueue_->Schedule (jid, prio, report);
	}

	GlooxCLEntry* ClientConnection::CreateCLEntry (const QString& jid)
	{
		return CreateCLEntry (Exts ().Get<QXmppRosterManager> ().getRosterEntry (jid));
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
				ScheduleFetchVCard (bareJID, false);
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
