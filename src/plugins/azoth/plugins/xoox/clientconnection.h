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
#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QXmppClient.h>
#include <QXmppMucIq.h>
#include <interfaces/imessage.h>
#include "glooxclentry.h"
#include "glooxaccount.h"

class QXmppMessage;
class QXmppMucManager;
class QXmppClient;
class QXmppDiscoveryManager;
class QXmppTransferManager;
class QXmppDiscoveryIq;

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

	class ClientConnection : public QObject
	{
		Q_OBJECT

		QXmppClient *Client_;
		QXmppMucManager *MUCManager_;
		QXmppTransferManager *XferManager_;
		QXmppDiscoveryManager *DiscoveryManager_;

		QString OurJID_;

		GlooxAccount *Account_;
		IProxyObject *ProxyObject_;

		QHash<QString, GlooxCLEntry*> JID2CLEntry_;
		QHash<QString, GlooxCLEntry*> ODSEntries_;

		bool IsConnected_;
		bool FirstTimeConnect_;

		QHash<QString, RoomHandler*> RoomHandlers_;
		GlooxAccountState LastState_;
		QString Password_;
		
		int SocketErrorAccumulator_;
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
		QXmppTransferManager* GetTransferManager () const;

		void RequestInfo (const QString&) const;

		void Update (const QXmppRosterIq::Item&);
		void Update (const QXmppMucAdminIq::Item&, const QString& room);

		void AckAuth (QObject*, bool);
		void Subscribe (const QString&,
				const QString& = QString (), const QString& = QString (),
				const QStringList& = QStringList ());
		void RevokeSubscription (const QString&, const QString&);
		void Unsubscribe (const QString&, const QString&);
		void Remove (GlooxCLEntry*);

		QXmppClient* GetClient () const;
		QObject* GetCLEntry (const QString& bareJid, const QString& variant) const;
		GlooxCLEntry* AddODSCLEntry (GlooxCLEntry::OfflineDataSource_ptr);
		QList<QObject*> GetCLEntries () const;
		void FetchVCard (const QString&);
		GlooxMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&, const QXmppRosterIq::Item&);

		static void Split (const QString& full,
				QString *bare, QString *resource);
	private:
		EntryStatus PresenceToStatus (const QXmppPresence&) const;
		void HandleOtherPresence (const QXmppPresence&);
		void HandleError (const QXmppIq&);
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
		void handleInfoReceived (const QXmppDiscoveryIq&);
		void handleItemsReceived (const QXmppDiscoveryIq&);
		void handlePresenceChanged (const QXmppPresence&);
		void handleRoomPresenceChanged (const QString&,
				const QString&, const QXmppPresence&);
		void handleMessageReceived (const QXmppMessage&);
		void handleRoomPermissionsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&);
		void handleRoomPartNickChange (const QString&, const QString&, const QString&);
		void handleRoomParticipantPermsChanged (const QString&, const QString&,
				QXmppMucAdminIq::Item::Affiliation,
				QXmppMucAdminIq::Item::Role,
				const QString&);
		
		void decrementErrAccumulators ();
	private:
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
