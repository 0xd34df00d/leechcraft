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
#include <gloox/mucroom.h>
#include <gloox/messagesession.h>
#include <gloox/client.h>
#include "glooxaccount.h"
#include "roomclentry.h"
#include "roompublicmessage.h"
#include "roomparticipantentry.h"
#include "clientconnection.h"
#include "glooxmessage.h"

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
						const QString& nick = NickFromJID (msg.from ());
						RoomParticipantEntry *entry = GetParticipantEntry (nick);

						if (priv)
						{
							gloox::MessageSession *session = GetSessionWith (msg.from ());
							GlooxMessage *message = new GlooxMessage (msg,
									entry, session);
							message->SetDateTime (QDateTime::currentDateTime ());
							entry->HandleMessage (message);;
						}
						else
						{
							RoomPublicMessage *message =
									new RoomPublicMessage (msg, CLEntry_, entry);
							CLEntry_->HandleMessage (message);
						}
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

					void RoomHandler::handleMessage (const gloox::Message& msg, gloox::MessageSession *session)
					{
						const gloox::JID& from = msg.from ();
						const QString& nick = NickFromJID (from);
						RoomParticipantEntry *entry = GetParticipantEntry (nick);

						GlooxMessage *message = new GlooxMessage (msg,
								entry, session);
						entry->HandleMessage (message);
					}

					GlooxMessage* RoomHandler::CreateMessage (IMessage::MessageType type,
							const QString& nick, const QString& body)
					{
						GlooxMessage *message = new GlooxMessage (IMessage::MTChat,
								IMessage::DOut,
								GetParticipantEntry (nick),
								GetSessionWith (JIDForNick (nick)));
						message->SetBody (body);
						message->SetDateTime (QDateTime::currentDateTime ());
						return message;
					}

					RoomParticipantEntry* RoomHandler::CreateParticipantEntry (const QString& nick)
					{
						RoomParticipantEntry *entry = new RoomParticipantEntry (nick,
								this, Account_);
						Nick2Entry_ [nick] = entry;
						return entry;
					}

					RoomParticipantEntry* RoomHandler::GetParticipantEntry (const QString& nick)
					{
						if (!Nick2Entry_.contains (nick))
						{
							RoomParticipantEntry *entry = CreateParticipantEntry (nick);
							Account_->handleGotRosterItems (QList<QObject*> () << entry);
							return entry;
						}
						else
							return Nick2Entry_ [nick];
					}

					gloox::MessageSession* RoomHandler::GetSessionWith (const gloox::JID& with)
					{
						if (!JID2Session_.contains (with))
						{
							gloox::MessageSession *session =
									new gloox::MessageSession (Account_->GetClientConnection ()->GetClient (),
											with);
							session->registerMessageHandler (this);
							JID2Session_ [with] = session;
							return session;
						}
						else
							return JID2Session_ [with];
					}

					QString RoomHandler::NickFromJID (const gloox::JID& jid) const
					{
						return QString::fromUtf8 (jid.resource ().c_str ());
					}

					gloox::JID RoomHandler::JIDForNick (const QString& nick) const
					{
						gloox::MUCRoom *room = CLEntry_->GetRoom ();
						return gloox::JID (room->name () + "@" +
								room->service () + "/" +
								nick.toUtf8 ().constData ());
					}
				}
			}
		}
	}
}
