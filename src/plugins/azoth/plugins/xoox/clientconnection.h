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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QXmppClient.h>
#include <QXmppMucIq.h>
#include <QXmppPubSubIq.h>
#include <interfaces/imessage.h>
#include "glooxclentry.h"
#include "glooxaccount.h"

class QXmppMessage;
class QXmppMucManager;
class QXmppClient;
class QXmppDiscoveryManager;
class QXmppTransferManager;
class QXmppDiscoveryIq;
class QXmppBookmarkManager;
class QXmppArchiveManager;
class QXmppEntityTimeManager;
class QXmppPubSubManager;
class QXmppDeliveryReceiptsManager;

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
	class CapsManager;
	class AnnotationsManager;
	class FetchQueue;

	class ClientConnection : public QObject
	{
		Q_OBJECT

		QXmppClient *Client_;
		QXmppMucManager *MUCManager_;
		QXmppTransferManager *XferManager_;
		QXmppDiscoveryManager *DiscoveryManager_;
		QXmppBookmarkManager *BMManager_;
		QXmppEntityTimeManager *EntityTimeManager_;
		QXmppArchiveManager *ArchiveManager_;
		QXmppPubSubManager *PubSubManager_;
		QXmppDeliveryReceiptsManager *DeliveryReceiptsManager_;
		
		AnnotationsManager *AnnotationsManager_;

		QString OurJID_;

		GlooxAccount *Account_;
		IProxyObject *ProxyObject_;
		CapsManager *CapsManager_;

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
		
		int SocketErrorAccumulator_;
		
		QList<QXmppMessage> OfflineMsgQueue_;
		
		QHash<QString, QPointer<VCardDialog> > AwaitingVCardDialogs_;
		
		QHash<QString, QPointer<GlooxMessage> > UndeliveredMessages_;
	public:
		typedef boost::function<void (const QXmppDiscoveryIq&)> DiscoCallback_t;
	private:
		QHash<QString, DiscoCallback_t> AwaitingDiscoInfo_;
		QHash<QString, DiscoCallback_t> AwaitingDiscoItems_;
		
		typedef QPair<QPointer<QObject>, QByteArray> PacketCallback_t;
		typedef QHash<QString, PacketCallback_t> PacketID2Callback_t;
		QHash<QString, PacketID2Callback_t> AwaitingPacketCallbacks_;
	public:
		ClientConnection (const QString&,
				const GlooxAccountState&,
				GlooxAccount*);
		virtual ~ClientConnection ();

		void SetState (const GlooxAccountState&);
		void Synchronize ();

		void SetPassword (const QString&);

		QString GetOurJID () const;
		void SetOurJID (const QString&);

		/** Joins the room and returns the contact list
		 * entry representing that room.
		 */
		RoomCLEntry* JoinRoom (const QString& room, const QString& user);
		void Unregister (RoomHandler*);

		QXmppMucManager* GetMUCManager () const;
		QXmppDiscoveryManager* GetDiscoveryManager () const;
		QXmppTransferManager* GetTransferManager () const;
		CapsManager* GetCapsManager () const;
		AnnotationsManager* GetAnnotationsManager () const;

		void RequestInfo (const QString&) const;
		
		void RequestInfo (const QString&, DiscoCallback_t, const QString& = "");
		void RequestItems (const QString&, DiscoCallback_t, const QString& = "");

		void Update (const QXmppRosterIq::Item&);
		void Update (const QXmppMucItem&, const QString& room);

		void AckAuth (QObject*, bool);
		void AddEntry (const QString&, const QString&, const QStringList&);
		void Subscribe (const QString&,
				const QString& = QString (), const QString& = QString (),
				const QStringList& = QStringList ());
		void RevokeSubscription (const QString&, const QString&);
		void Unsubscribe (const QString&, const QString&);
		void Remove (GlooxCLEntry*);

		void SendPacketWCallback (const QXmppIq&, QObject*, const QByteArray&);
		void SendMessage (GlooxMessage*);
		QXmppClient* GetClient () const;
		QObject* GetCLEntry (const QString& bareJid, const QString& variant) const;
		GlooxCLEntry* AddODSCLEntry (GlooxCLEntry::OfflineDataSource_ptr);
		QList<QObject*> GetCLEntries () const;
		void FetchVCard (const QString&);
		void FetchVCard (const QString&, VCardDialog*);
		QXmppBookmarkSet GetBookmarks () const;
		void SetBookmarks (const QXmppBookmarkSet&);
		GlooxMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&, const QXmppRosterIq::Item&);

		static void Split (const QString& full,
				QString *bare, QString *resource);
	private:
		EntryStatus PresenceToStatus (const QXmppPresence&) const;
		void HandleOtherPresence (const QXmppPresence&);
		void HandleError (const QXmppIq&);
		void InvokeCallbacks (const QXmppIq&);
		QString HandleErrorCondition (const QXmppStanza::Error::Condition&);
	private slots:
		void handleConnected ();
		void handleReconnecting (int = -1);
		void handleError (QXmppClient::Error);
		void handleIqReceived (const QXmppIq&);
		void handleRosterReceived ();
		void handleRosterChanged (const QString&);
		void handleRosterItemRemoved (const QString&);
		void handleVCardReceived (const QXmppVCardIq&);
		void handlePresenceChanged (const QXmppPresence&);
		void handleMessageReceived (const QXmppMessage&);
		void handleMessageDelivered (const QString&);
		
		void handleBookmarksReceived (const QXmppBookmarkSet&);
		void handleAutojoinQueue ();
		
		void handleDiscoInfo (const QXmppDiscoveryIq&);
		void handleDiscoItems (const QXmppDiscoveryIq&);
		
		void decrementErrAccumulators ();
	private:
		void ScheduleFetchVCard (const QString&);
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

		void serverAuthFailed ();
		void needPassword ();
		void statusChanged (const EntryStatus&);
	};
}
}
}

#endif
