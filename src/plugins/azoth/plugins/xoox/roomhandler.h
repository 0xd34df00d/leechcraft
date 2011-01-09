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
#include <gloox/mucroomhandler.h>
#include <gloox/messagehandler.h>
#include "clientconnection.h"
#include "roomparticipantentry.h"

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
	class RoomCLEntry;
	class GlooxAccount;
	class RoomParticipantEntry;

	class RoomHandler : public QObject
					  , public gloox::MUCRoomHandler
					  , public gloox::MessageHandler
	{
		Q_OBJECT

		GlooxAccount *Account_;
		RoomCLEntry *CLEntry_;
		QHash<QString, RoomParticipantEntry_ptr> Nick2Entry_;
		QHash<gloox::JID, gloox::MessageSession*> JID2Session_;
		boost::shared_ptr<gloox::MUCRoom> Room_;
		QString Subject_;
		bool RoomHasBeenEntered_;
	public:
		RoomHandler (GlooxAccount*);

		/** This must be called before any calls to
		 * GetCLEntry().
		 */
		void SetRoom (boost::shared_ptr<gloox::MUCRoom>);
		boost::shared_ptr<gloox::MUCRoom> GetRoom () const;
		gloox::JID GetRoomJID () const;
		RoomCLEntry* GetCLEntry ();
		void HandleVCard (const gloox::VCard*, const QString&);

		GlooxMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QObject*> GetParticipants () const;
		QString GetSubject () const;
		void Kick (const QString& nick, const QString& reason = QString ());
		void Leave (const QString& msg);

		// MUCRoomHandler
		virtual void handleMUCParticipantPresence (gloox::MUCRoom*,
				const gloox::MUCRoomParticipant, const gloox::Presence&);
		virtual void handleMUCMessage (gloox::MUCRoom*,
				const gloox::Message&, bool);
		virtual bool handleMUCRoomCreation (gloox::MUCRoom*);
		virtual void handleMUCSubject (gloox::MUCRoom*,
				const std::string&, const std::string&);
		virtual void handleMUCInviteDecline (gloox::MUCRoom*,
				const gloox::JID&, const std::string&);
		virtual void handleMUCError (gloox::MUCRoom*, gloox::StanzaError);
		virtual void handleMUCInfo (gloox::MUCRoom*, int,
				const std::string&, const gloox::DataForm*);
		virtual void handleMUCItems (gloox::MUCRoom*,
				const gloox::Disco::ItemList&);

		// MessageHandler
		virtual void handleMessage (const gloox::Message&, gloox::MessageSession*);
	private:
		/** Creates a new entry for the given nick.
		 */
		RoomParticipantEntry_ptr CreateParticipantEntry (const QString& nick, bool announce);

		/** Creates a new entry for the given nick if it
		 * doesn't exist already (and does so by calling
		 * CreateParticipantEntry()) or just returns the
		 * already existing one.
		 */
		RoomParticipantEntry_ptr GetParticipantEntry (const QString& nick, bool announce = true);

		void MakeLeaveMessage (const gloox::MUCRoomParticipant);
		void MakeStatusChangedMessage (const gloox::MUCRoomParticipant, const gloox::Presence&);
		void MakeJoinMessage (const gloox::MUCRoomParticipant);

		gloox::MessageSession* GetSessionWith (const gloox::JID&);
		QString NickFromJID (const gloox::JID&) const;
		gloox::JID JIDForNick (const QString&) const;

		void RemoveThis ();
	};
}
}
}
}
}

#endif
