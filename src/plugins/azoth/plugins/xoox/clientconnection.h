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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#include <functional>
#include <memory>
#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QXmppClient.h>
#include <QXmppMucIq.h>
#include <interfaces/azoth/imessage.h>
#include "glooxclentry.h"
#include "glooxaccount.h"
#include "riexmanager.h"

class QXmppMessage;
class QXmppMucManager;
class QXmppClient;
class QXmppDiscoveryManager;
class QXmppTransferManager;
class QXmppDiscoveryIq;
class QXmppBookmarkManager;
class QXmppArchiveManager;
class QXmppEntityTimeManager;
class QXmppMessageReceiptManager;
#ifdef ENABLE_MEDIACALLS
class QXmppCallManager;
class QXmppCall;
#endif

namespace LeechCraft
{
struct Entity;

namespace Azoth
{
class IProxyObject;

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
	class AdHocCommandManager;
	class LastActivityManager;
	class JabberSearchManager;
	class XMPPBobManager;
	class XMPPCaptchaManager;
	class UserAvatarManager;
	class MsgArchivingManager;
	class SDManager;

	class ClientConnectionErrorMgr;
	class CryptHandler;
	class ServerInfoStorage;

	class ClientConnection : public QObject
	{
		Q_OBJECT

		GlooxAccount *Account_;
		AccountSettingsHolder *Settings_;

		QXmppClient *Client_;
		QXmppMucManager *MUCManager_;
		QXmppTransferManager *XferManager_;
		QXmppDiscoveryManager *DiscoveryManager_;
		QXmppBookmarkManager *BMManager_;
		QXmppEntityTimeManager *EntityTimeManager_;
		QXmppArchiveManager *ArchiveManager_;
		QXmppMessageReceiptManager *DeliveryReceiptsManager_;
		XMPPCaptchaManager *CaptchaManager_;
		XMPPBobManager *BobManager_;
#ifdef ENABLE_MEDIACALLS
		QXmppCallManager *CallManager_;
#endif
		PubSubManager *PubSubManager_;
		PrivacyListsManager *PrivacyListsManager_;
		AdHocCommandManager *AdHocCommandManager_;
		AnnotationsManager *AnnotationsManager_;
		LastActivityManager *LastActivityManager_;
		JabberSearchManager *JabberSearchManager_;
		UserAvatarManager *UserAvatarManager_;
		RIEXManager *RIEXManager_;
		MsgArchivingManager *MsgArchivingManager_;
		SDManager *SDManager_;

		CryptHandler *CryptHandler_;
		ClientConnectionErrorMgr *ErrorMgr_;

		QString OurJID_;
		QString OurBareJID_;
		QString OurResource_;

		SelfContact *SelfContact_;

		IProxyObject *ProxyObject_;
		CapsManager *CapsManager_;

		ServerInfoStorage *ServerInfoStorage_;

		QHash<QString, GlooxCLEntry*> JID2CLEntry_;
		QHash<QString, GlooxCLEntry*> ODSEntries_;

		bool IsConnected_;
		bool FirstTimeConnect_;

		QHash<QString, RoomHandler*> RoomHandlers_;
		GlooxAccountState LastState_;
		QString Password_;

		struct JoinQueueItem
		{
			QString RoomJID_;
			QString Nickname_;
		};
		QList<JoinQueueItem> JoinQueue_;

		FetchQueue *VCardQueue_;
		FetchQueue *CapsQueue_;
		FetchQueue *VersionQueue_;

		QList<QXmppMessage> OfflineMsgQueue_;
		QList<QPair<QString, PEPEventBase*>> InitialEventQueue_;
		QHash<QString, QPointer<GlooxMessage>> UndeliveredMessages_;

		QHash<QString, QList<RIEXManager::Item>> AwaitingRIEXItems_;
	public:
		typedef std::function<void (const QXmppDiscoveryIq&)> DiscoCallback_t;
		typedef std::function<void (const QXmppVCardIq&)> VCardCallback_t;
	private:
		QHash<QString, DiscoCallback_t> AwaitingDiscoInfo_;
		QHash<QString, DiscoCallback_t> AwaitingDiscoItems_;

		typedef QPair<QPointer<QObject>, QByteArray> PacketCallback_t;
		typedef QHash<QString, PacketCallback_t> PacketID2Callback_t;
		QHash<QString, PacketID2Callback_t> AwaitingPacketCallbacks_;

		QHash<QString, QList<VCardCallback_t>> VCardFetchCallbacks_;
	public:
		ClientConnection (GlooxAccount*);
		virtual ~ClientConnection ();

		void SetState (const GlooxAccountState&);
		GlooxAccountState GetLastState () const;

		void SetPassword (const QString&);

		QString GetOurJID () const;
		void SetOurJID (const QString&);

		/** Joins the room and returns the contact list
		 * entry representing that room.
		 */
		RoomCLEntry* JoinRoom (const QString& room, const QString& user);
		void Unregister (RoomHandler*);

		void CreateEntry (const QString&);

		QXmppMucManager* GetMUCManager () const;
		QXmppDiscoveryManager* GetDiscoveryManager () const;
		QXmppVersionManager* GetVersionManager () const;
		QXmppTransferManager* GetTransferManager () const;
		CapsManager* GetCapsManager () const;
		AnnotationsManager* GetAnnotationsManager () const;
		PubSubManager* GetPubSubManager () const;
		PrivacyListsManager* GetPrivacyListsManager () const;
		XMPPBobManager* GetBobManager () const;
#ifdef ENABLE_MEDIACALLS
		QXmppCallManager* GetCallManager () const;
#endif
		AdHocCommandManager* GetAdHocCommandManager () const;
		JabberSearchManager* GetJabberSearchManager () const;
		UserAvatarManager* GetUserAvatarManager () const;
		RIEXManager* GetRIEXManager () const;
		SDManager* GetSDManager () const;

		CryptHandler* GetCryptHandler () const;
		ServerInfoStorage* GetServerInfoStorage () const;

		void SetSignaledLog (bool);

		void RequestInfo (const QString&) const;

		void RequestInfo (const QString&, DiscoCallback_t, bool reportErrors = false, const QString& = "");
		void RequestItems (const QString&, DiscoCallback_t, bool reportErrors = false, const QString& = "");

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

		void WhitelistError (const QString&);

		void SendPacketWCallback (const QXmppIq&, QObject*, const QByteArray&);
		void SendMessage (GlooxMessage*);
		QXmppClient* GetClient () const;
		QObject* GetCLEntry (const QString& fullJid) const;
		QObject* GetCLEntry (const QString& bareJid, const QString& variant) const;
		GlooxCLEntry* AddODSCLEntry (OfflineDataSource_ptr);
		QList<QObject*> GetCLEntries () const;
		void FetchVCard (const QString&, bool reportErrors = false);
		void FetchVCard (const QString&, VCardCallback_t, bool reportErrors = false);
		void FetchVersion (const QString&, bool reportErrors = false);
		QXmppBookmarkSet GetBookmarks () const;
		void SetBookmarks (const QXmppBookmarkSet&);
		GlooxMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&, const QString&);

		static void Split (const QString& full,
				QString *bare, QString *resource);
	private:
		void SetupLogger ();
		void HandleOtherPresence (const QXmppPresence&);
		void HandleRIEX (QString, QList<RIEXManager::Item>, QString = QString ());
		void InvokeCallbacks (const QXmppIq&);
	public slots:
		void handlePendingForm (QXmppDataForm*, const QString&);
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
		void handleMessageReceived (QXmppMessage);
		void handlePEPEvent (const QString&, PEPEventBase*);
		void handlePEPAvatarUpdated (const QString&, const QImage&);
		void handleMessageDelivered (const QString&, const QString&);
		void handleCaptchaReceived (const QString&, const QXmppDataForm&);
		void handleRoomInvitation (const QString&, const QString&, const QString&);
		void handleGotRIEXItems (QString, QList<RIEXManager::Item>, bool);

		void handleBookmarksReceived (const QXmppBookmarkSet&);
		void handleAutojoinQueue ();

		void handleDiscoInfo (const QXmppDiscoveryIq&);
		void handleDiscoItems (const QXmppDiscoveryIq&);

		void handleLog (QXmppLogger::MessageType, const QString&);

		void setKAParams (const QPair<int, int>&);
		void setFileLogging (bool);
		void handlePhotoHash ();
		void handlePriorityChanged (int);
		void updateFTSettings ();
		void handleDetectedBSProxy (const QString&);

		void handleVersionSettingsChanged ();
	private:
		void ScheduleFetchVCard (const QString&, bool);
		GlooxCLEntry* CreateCLEntry (const QString&);
		GlooxCLEntry* CreateCLEntry (const QXmppRosterIq::Item&);
		GlooxCLEntry* ConvertFromODS (const QString&, const QXmppRosterIq::Item&);
	signals:
		void gotRosterItems (const QList<QObject*>&);
		void rosterItemRemoved (QObject*);
		void rosterItemsRemoved (const QList<QObject*>&);
		void rosterItemUpdated (QObject*);
		void rosterItemSubscribed (QObject*, const QString&);
		void rosterItemUnsubscribed (QObject*, const QString&);
		void rosterItemUnsubscribed (const QString&, const QString&);
		void rosterItemCancelledSubscription (QObject*, const QString&);
		void rosterItemGrantedSubscription (QObject*, const QString&);
		void gotSubscriptionRequest (QObject*, const QString&);
		void gotMUCInvitation (const QVariantMap&, const QString&, const QString&);

		void gotConsoleLog (const QByteArray&, int, const QString&);

		void gotRequestedPosts (const QList<LeechCraft::Azoth::Post>&, const QString&);
		void gotNewPost (const LeechCraft::Azoth::Post&);

		void serverAuthFailed ();
		void needPassword ();
		void statusChanged (const EntryStatus&);
	};
}
}
}

#endif
