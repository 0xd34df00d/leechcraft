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

#include "roomhandler.h"
#include <QtDebug>
#include "glooxaccount.h"
#include "roomclentry.h"
#include "roompublicmessage.h"
#include "roomparticipantentry.h"
#include <gloox/mucroom.h>

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
					RoomHandler::RoomHandler (GlooxAccount* account)
					: QObject (account)
					, Account_ (account)
					, CLEntry_ (0)
					{
					}

					void RoomHandler::SetRoom (gloox::MUCRoom *room)
					{
						CLEntry_ = new RoomCLEntry (room, Account_);
						room->getRoomItems ();
					}

					RoomCLEntry* RoomHandler::GetCLEntry ()
					{
						return CLEntry_;
					}

					void RoomHandler::handleMUCParticipantPresence (gloox::MUCRoom *room,
							const gloox::MUCRoomParticipant participant, const gloox::Presence& presence)
					{
					}

					void RoomHandler::handleMUCMessage (gloox::MUCRoom *room, const gloox::Message& msg, bool priv)
					{
						// TODO
						if (priv)
						{
							qWarning () << Q_FUNC_INFO
									<< "we don't support private msgs yet";
							return;
						}

						const QString nick = NickFromJID (msg.from ());
						if (!Nick2Entry_.contains (nick))
						{
							RoomParticipantEntry *entry = CreateParticipantEntry (nick);
							Account_->handleGotRosterItems (QList<QObject*> () << entry);
						}
						RoomPublicMessage *message = new RoomPublicMessage (msg, CLEntry_, Nick2Entry_ [nick]);
						CLEntry_->HandleMessage (message);
					}

					bool RoomHandler::handleMUCRoomCreation (gloox::MUCRoom *room)
					{
						return true;
					}

					void RoomHandler::handleMUCSubject (gloox::MUCRoom *room, const std::string& nick, const std::string& subject)
					{
					}

					void RoomHandler::handleMUCInviteDecline (gloox::MUCRoom* room, const gloox::JID& invitee, const std::string&reason)
					{
					}

					void RoomHandler::handleMUCError (gloox::MUCRoom* room, gloox::StanzaError error)
					{
					}

					void RoomHandler::handleMUCInfo (gloox::MUCRoom *room, int features, const std::string& name, const gloox::DataForm *infoForm)
					{
					}

					void RoomHandler::handleMUCItems (gloox::MUCRoom *room, const gloox::Disco::ItemList& items)
					{
						QList<QObject*> entries;
						Q_FOREACH (gloox::Disco::Item *item, items)
						{
							const QString nick = QString::fromUtf8 (item->name ().c_str ());
							entries << CreateParticipantEntry (nick);
						}
						Account_->handleGotRosterItems (entries);
					}

					RoomParticipantEntry* RoomHandler::CreateParticipantEntry (const QString& nick)
					{
						RoomParticipantEntry *entry = new RoomParticipantEntry (nick,
								CLEntry_->GetRoom (), Account_);
						Nick2Entry_ [nick] = entry;
						return entry;
					}

					QString RoomHandler::NickFromJID (const gloox::JID& jid) const
					{
						return QString::fromUtf8 (jid.resource ().c_str ());
					}
				}
			}
		}
	}
}
