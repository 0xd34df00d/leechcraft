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

#include "roomparticipantentry.h"
#include <QtDebug>
#include <gloox/mucroom.h>
#include "glooxaccount.h"
#include "roompublicmessage.h"

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
					RoomParticipantEntry::RoomParticipantEntry (const QString& nick,
							gloox::MUCRoom *room, GlooxAccount *account)
					: QObject (account)
					, Nick_ (nick)
					, Account_ (account)
					, Room_ (room)
					{
					}

					QObject* RoomParticipantEntry::GetObject ()
					{
						return this;
					}

					IAccount* RoomParticipantEntry::GetParentAccount () const
					{
						return Account_;
					}

					ICLEntry::Features RoomParticipantEntry::GetEntryFeatures () const
					{
						return FIsPrivateChat | FSessionEntry;
					}

					QString RoomParticipantEntry::GetEntryName () const
					{
						return Nick_;
					}

					void RoomParticipantEntry::SetEntryName (const QString&)
					{
					}

					QByteArray RoomParticipantEntry::GetEntryID () const
					{
						return (Room_->service () + "/" +
								Room_->name () + "/" +
								Nick_.toUtf8 ().constData ()).c_str ();
					}

					QStringList RoomParticipantEntry::Groups () const
					{
						return QStringList (tr ("%1@%2 participants")
								.arg (QString::fromUtf8 (Room_->name ().c_str ())
								.arg (QString::fromUtf8 (Room_->service ().c_str ()))));
					}

					QStringList RoomParticipantEntry::Variants () const
					{
						return QStringList ("");
					}

					IMessage* RoomParticipantEntry::CreateMessage (IMessage::MessageType type,
							const QString& variant, const QString& body)
					{
						qWarning () << Q_FUNC_INFO << "not implemented, gonna crash now";
						return 0;
					}

					QList<IMessage*> RoomParticipantEntry::GetAllMessages () const
					{
						return AllMessages_;
					}
				}
			}
		}
	}
}
