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

#include "roomclentry.h"
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
					RoomCLEntry::RoomCLEntry (gloox::MUCRoom *room, GlooxAccount *account)
					: QObject (account)
					, Account_ (account)
					, Room_ (room)
					{
					}

					QObject* RoomCLEntry::GetObject ()
					{
						return this;
					}

					IAccount* RoomCLEntry::GetParentAccount () const
					{
						return Account_;
					}

					ICLEntry::Features RoomCLEntry::GetEntryFeatures () const
					{
						return FSessionEntry |
							FIsMUC;
					}

					QString RoomCLEntry::GetEntryName () const
					{
						return QString::fromUtf8 (Room_->name ().c_str ());
					}

					void RoomCLEntry::SetEntryName (const QString&)
					{
					}

					QByteArray RoomCLEntry::GetEntryID () const
					{
						return (Room_->service () + "/" +
									Room_->name () + "/" +
									Room_->nick ()).c_str ();
					}

					QStringList RoomCLEntry::Groups () const
					{
						return QStringList () << tr ("Multiuser chatrooms");
					}

					QStringList RoomCLEntry::Variants () const
					{
						QStringList result;
						result << "";
						return result;
					}

					IMessage* RoomCLEntry::CreateMessage (IMessage::MessageType type,
							const QString& variant, const QString& text)
					{
						if (variant == "")
							return new RoomPublicMessage (text, this);
						else
							return 0;
					}

					QList<IMessage*> RoomCLEntry::GetAllMessages () const
					{
						return AllMessages_;
					}

					EntryStatus RoomCLEntry::GetStatus () const
					{
						return EntryStatus ();
					}

					gloox::MUCRoom* RoomCLEntry::GetRoom ()
					{
						return Room_;
					}

					void RoomCLEntry::HandleMessage (RoomPublicMessage *msg)
					{
						AllMessages_ << msg;
						emit gotMessage (msg);
					}
				}
			}
		}
	}
}
