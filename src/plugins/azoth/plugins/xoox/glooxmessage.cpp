/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "glooxmessage.h"
#include <gloox/messagesession.h>
#include "glooxclentry.h"

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
					GlooxMessage::GlooxMessage (IMessage::MessageType type,
							IMessage::Direction direction,
							GlooxCLEntry *entry,
							const QString& variant,
							gloox::MessageSession *session)
					: Type_ (type)
					, Direction_ (direction)
					, Entry_ (entry)
					, Variant_ (variant)
					, Session_ (session)
					{
					}

					QObject* GlooxMessage::GetObject ()
					{
						return this;
					}

					void GlooxMessage::Send ()
					{
						switch (Type_)
						{
						case MTChat:
						case MTGroupchat:
							Session_->send (Body_.toUtf8 ().constData (), std::string ());
							return;
						}
					}

					IMessage::Direction GlooxMessage::GetDirection () const
					{
						return Direction_;
					}

					IMessage::MessageType GlooxMessage::GetMessageType () const
					{
						return Type_;
					}

					ICLEntry* GlooxMessage::OtherPart () const
					{
						return Entry_;
					}

					QString GlooxMessage::GetOtherVariant () const
					{
						return Variant_;
					}

					QString GlooxMessage::GetBody () const
					{
						return Body_;
					}

					void GlooxMessage::SetBody (const QString& body)
					{
						Body_ = body;
					}
				}
			}
		}
	}
}
