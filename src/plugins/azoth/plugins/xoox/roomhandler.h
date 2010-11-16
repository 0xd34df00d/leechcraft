/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
						QHash<QString, RoomParticipantEntry*> Nick2Entry_;
						QHash<gloox::JID, gloox::MessageSession*> JID2Session_;
						gloox::MUCRoom *Room_;
					public:
						RoomHandler (GlooxAccount* = 0);

						/** This must be called before any calls to
						 * GetCLEntry().
						 */
						void SetRoom (gloox::MUCRoom*);
						gloox::MUCRoom* GetRoom () const;
						RoomCLEntry* GetCLEntry ();

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

						GlooxMessage* CreateMessage (IMessage::MessageType,
								const QString&, const QString&);
						QList<ICLEntry*> GetParticipants () const;
					private:
						/** Creates a new entry for the given nick.
						 */
						RoomParticipantEntry* CreateParticipantEntry (const QString& nick);
						/** Creates a new entry for the given nick if it
						 * doesn't exist already (and does so by calling
						 * CreateParticipantEntry()) or just returns the
						 * already existing one.
						 */
						RoomParticipantEntry* GetParticipantEntry (const QString& nick);
						gloox::MessageSession* GetSessionWith (const gloox::JID&);
						QString NickFromJID (const gloox::JID&) const;
						gloox::JID JIDForNick (const QString&) const;
					};
				}
			}
		}
	}
}

#endif
