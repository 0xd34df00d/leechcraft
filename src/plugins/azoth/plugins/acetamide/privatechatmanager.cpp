/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "privatechatmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	PrivateChatManager::PrivateChatManager (QObject *parent)
	: QObject (parent)
	{
	}

	PrivateChatEntry_ptr PrivateChatManager::GetChatEntry (const QString& nick,
			IrcServer *srv, IrcAccount *acc)
	{
		if (!Nick2Entry.contains (nick))
		{
			PrivateChatEntry_ptr entry (CreateNewChatEntry (nick, srv, acc));
			Nick2Entry [nick] = entry;
			return entry;
		}
		else
			return Nick2Entry.value (nick);
	}

	PrivateChatEntry_ptr PrivateChatManager::CreateNewChatEntry (const QString& nick, IrcServer *srv, IrcAccount *acc)
	{
		PrivateChatEntry_ptr entry (new PrivateChatEntry (nick, srv, acc));
		Nick2Entry [nick] = entry;
		return entry;
	}

};
};
};
