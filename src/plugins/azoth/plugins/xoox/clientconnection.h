/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QXmppClient.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/icanhavesslerrors.h>
#include "glooxclentry.h"
#include "glooxaccount.h"

class QXmppMessage;
class QXmppMucManager;
class QXmppClient;
class QXmppDiscoveryManager;
class QXmppDiscoveryIq;

namespace LC
{
struct Entity;

namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class GlooxMessage;
	class RoomCLEntry;
	class RoomHandler;
	class SelfContact;
	class CapsManager;
	class AnnotationsManager;
	class FetchQueue;
	class PubSubManager;
	class PrivacyListsManager;
	class UserAvatarManager;
	class SDManager;
	class Xep0313Manager;
	class CarbonsManager;

	class InfoRequestPolicyManager;
	class ClientConnectionErrorMgr;
	class CryptHandler;
	class ServerInfoStorage;

	class DiscoManagerWrapper;

	class ClientConnectionExtensionsManager;

	class ClientConnection : public QObject
	{
		Q_OBJECT

		GlooxAccount *Account_;
		AccountSettingsHolder *Settings_;

		QXmppClient *Client_;

		QXmppMucManager *MUCManager_;
		QXmppDiscoveryManager *DiscoveryManager_;

		PubSubManager *PubSubManager_;
		PrivacyListsManager *PrivacyListsManager_;
		AnnotationsManager *AnnotationsManager_;
		UserAvatarManager *UserAvatarManager_;
		SDManager *SDManager_;
		Xep0313Manager *Xep0313Manager_;
		CarbonsManager *CarbonsManager_;

		CryptHandler *CryptHandler_;
		ClientConnectionErrorMgr *ErrorMgr_;

		InfoRequestPolicyManager *InfoReqPolicyMgr_;

		DiscoManagerWrapper *DiscoManagerWrapper_;

		std::unique_ptr<ClientConnectionExtensionsManager> ExtsMgr_;

		QString OurJID_;
		QString OurBareJID_;
		QString OurResource_;

		SelfContact *SelfContact_;

		CapsManager *CapsManager_;

		ServerInfoStorage *ServerInfoStorage_;

		QHash<QString, GlooxCLEntry*> JID2CLEntry_;
		QHash<QString, GlooxCLEntry*> ODSEntries_;

		bool IsConnected_ = false;
		bool FirstTimeConnect_ = true;

		QHash<QString, RoomHandler*> RoomHandlers_;
		GlooxAccountState LastState_;
		QString Password_;

		FetchQueue *VCardQueue_;
		FetchQueue *CapsQueue_;
		FetchQueue *VersionQueue_;

		QList<QXmppMessage> OfflineMsgQueue_;
		QList<QPair<QString, PEPEventBase*>> InitialEventQueue_;
	public:
		typedef std::function<void (const QXmppVCardIq&)> VCardCallback_t;
		typedef std::function<void (QXmppIq)> PacketCallback_t;
	private:
		QHash<QString, PacketCallback_t> AwaitingPacketCallbacks_;

		QHash<QString, QList<VCardCallback_t>> VCardFetchCallbacks_;
	public:
		ClientConnection (GlooxAccount*);
		~ClientConnection ();

		void SetState (const GlooxAccountState&);
		GlooxAccountState GetLastState () const;
		bool IsConnected () const;

		void SetPassword (const QString&);

		QString GetOurJID () const;
		void SetOurJID (const QString&);

		QString GetOurResource () const;

		/** Joins the room and returns the contact list
		 * entry representing that room.
		 */
		RoomCLEntry* JoinRoom (const QString& room, const QString& user, bool asAutojoin = false);
		void Unregister (RoomHandler*);

		void CreateEntry (const QString&);

		ClientConnectionExtensionsManager& Exts () const;

		DiscoManagerWrapper* GetDiscoManagerWrapper () const;

		QXmppMucManager* GetMUCManager () const;
		QXmppDiscoveryManager* GetQXmppDiscoveryManager () const;
		QXmppVersionManager* GetVersionManager () const;
		CapsManager* GetCapsManager () const;
		AnnotationsManager* GetAnnotationsManager () const;
		PubSubManager* GetPubSubManager () const;
		PrivacyListsManager* GetPrivacyListsManager () const;
		UserAvatarManager* GetUserAvatarManager () const;
		SDManager* GetSDManager () const;
		Xep0313Manager* GetXep0313Manager () const;

		InfoRequestPolicyManager* GetInfoReqPolicyManager () const;

		CryptHandler* GetCryptHandler () const;
		ServerInfoStorage* GetServerInfoStorage () const;

		void RequestInfo (const QString&) const;

		void Update (const QXmppRosterIq::Item&);
		void Update (const QXmppMucItem&, const QString& room);

		void AckAuth (QObject*, bool);
		void AddEntry (const QString&, const QString&, const QStringList&);
		void Subscribe (const QString&,
				const QString& = QString (), const QString& = QString (),
				const QStringList& = QStringList ());
		void Unsubscribe (const QString&, const QString&);
		void GrantSubscription (const QString&, const QString&);
		void RevokeSubscription (const QString&, const QString&);
		void Remove (GlooxCLEntry*);

		ClientConnectionErrorMgr* GetErrorManager () const;

		void SendPacketWCallback (const QXmppIq&, PacketCallback_t);
		void AddCallback (const QString&, const PacketCallback_t&);
		void SendMessage (GlooxMessage*);

		GlooxAccount* GetAccount () const;
		QXmppClient* GetClient () const;
		QObject* GetCLEntry (const QString& fullJid) const;
		QObject* GetCLEntry (const QString& bareJid, const QString& variant) const;
		GlooxCLEntry* AddODSCLEntry (OfflineDataSource_ptr);
		QList<QObject*> GetCLEntries () const;
		void FetchVCard (const QString&, bool reportErrors = false);
		void FetchVCard (const QString&, VCardCallback_t, bool reportErrors = false);
		void FetchVersion (const QString&, bool reportErrors = false);

		GlooxMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&, const QString&);

		struct SplitResult
		{
			QString Bare_;
			QString Resource_;

			operator std::tuple<QString&, QString&> () &&
			{
				return std::tie (Bare_, Resource_);
			}
		};
		static SplitResult Split (const QString& full);
	private:
		void HandleOtherPresence (const QXmppPresence&);
		void InvokeCallbacks (const QXmppIq&);
	private slots:
		void handleConnected ();
		void handleDisconnected ();
		void handleIqReceived (const QXmppIq&);
		void handleRosterReceived ();
		void handleRosterChanged (const QString&);
		void handleRosterItemRemoved (const QString&);
		void handleVCardReceived (const QXmppVCardIq&);
		void handleVersionReceived (const QXmppVersionIq&);
		void handlePresenceChanged (const QXmppPresence&);

		void handleMessageReceived (QXmppMessage, bool forwarded = false);
		void handleCarbonsMessage (const QXmppMessage&);

		void handlePEPEvent (const QString&, PEPEventBase*);
		void handlePEPAvatarUpdated (const QString&);

		void handleRoomInvitation (const QString&, const QString&, const QString&);

		void setKAParams (const QPair<int, int>&);
		void handlePhotoHash ();
		void handlePriorityChanged (int);

		void handleMessageCarbonsSettingsChanged ();

		void handleVersionSettingsChanged ();
	private:
		void ScheduleFetchVCard (const QString&, bool);
		GlooxCLEntry* CreateCLEntry (const QString&);
		GlooxCLEntry* CreateCLEntry (const QXmppRosterIq::Item&);
		GlooxCLEntry* ConvertFromODS (const QString&, const QXmppRosterIq::Item&);
	signals:
		void gotRosterItems (const QList<QObject*>&);
		void rosterItemRemoved (QObject*);

		void gotRequestedPosts (const QList<LC::Azoth::Post>&, const QString&);
		void gotNewPost (const LC::Azoth::Post&);

		void serverAuthFailed ();
		void needPassword ();
		void statusChanged (const EntryStatus&);

		void connected ();

		void sslErrors (const QList<QSslError>&, const ICanHaveSslErrors::ISslErrorsReaction_ptr&);
	};
}
}
}
