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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMHANDLER_H
#include <QObject>
#include <QHash>
#include <interfaces/imucentry.h>
#include "clientconnection.h"
#include "roomparticipantentry.h"

class QXmppVCardIq;
class QXmppMucManager;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class RoomCLEntry;
	class GlooxAccount;
	class RoomParticipantEntry;

	class RoomHandler : public QObject
	{
		Q_OBJECT

		GlooxAccount *Account_;
		QXmppMucManager *MUCManager_;
		RoomCLEntry *CLEntry_;
		QHash<QString, RoomParticipantEntry_ptr> Nick2Entry_;
		QString Subject_;
		bool RoomHasBeenEntered_;
		QString RoomJID_;
		QString OurNick_;
		// contains new nicks
		QSet<QString> PendingNickChanges_;
	public:
		RoomHandler (const QString& roomJID, const QString& ourNick, GlooxAccount*);

		QString GetRoomJID () const;
		RoomCLEntry* GetCLEntry ();
		void HandleVCard (const QXmppVCardIq&, const QString&);

		void SetState (const GlooxAccountState&);

		GlooxMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QObject*> GetParticipants () const;
		QString GetSubject () const;
		void SetSubject (const QString&);
		void Leave (const QString& msg, bool remove = true);
		RoomParticipantEntry* GetSelf () const;
		QString GetOurNick () const;
		void SetOurNick (const QString&);

		void SetAffiliation (RoomParticipantEntry*,
				IMUCEntry::MUCAffiliation, const QString&);
		void SetRole (RoomParticipantEntry*,
				IMUCEntry::MUCRole, const QString&);

		void HandlePresence (const QXmppPresence&, const QString&);
		void HandleErrorPresence (const QXmppPresence&, const QString&);
		void HandlePermsChanged (const QString&,
				QXmppMucAdminIq::Item::Affiliation,
				QXmppMucAdminIq::Item::Role,
				const QString&);
		void HandleNickChange (const QString&, const QString&);
		void HandleMessage (const QXmppMessage&, const QString&);
		void UpdatePerms (const QList<QXmppMucAdminIq::Item>&);
		/** Creates a new entry for the given nick if it
		 * doesn't exist already (and does so by calling
		 * CreateParticipantEntry()) or just returns the
		 * already existing one.
		 */
		RoomParticipantEntry_ptr GetParticipantEntry (const QString& nick, bool announce = true);
	private:
		/** Creates a new entry for the given nick.
		 */
		RoomParticipantEntry_ptr CreateParticipantEntry (const QString& nick, bool announce);
		void MakeLeaveMessage (const QXmppPresence&, const QString&);
		void MakeJoinMessage (const QXmppPresence&, const QString&);
		void MakeStatusChangedMessage (const QXmppPresence&, const QString&);
		void MakeNickChangeMessage (const QString&, const QString&);
		void MakeKickMessage (const QString&, const QString&);
		void MakeBanMessage (const QString&, const QString&);
		void MakePermsChangedMessage (const QString&,
				QXmppMucAdminIq::Item::Affiliation,
				QXmppMucAdminIq::Item::Role,
				const QString&);
		/*
		void MakeRoleAffChangedMessage (const gloox::MUCRoomParticipant);
		*/

		void RemoveThis ();
	};
}
}
}

#endif
