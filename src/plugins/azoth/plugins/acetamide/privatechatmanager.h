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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_PRIVATECHATMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_PRIVATECHATMANAGER_H

#include <QObject>
#include <QHash>
#include "privatechatentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;
	class IrcServer;
	class IrcMessage;

	class PrivateChatManager : public QObject
	{
		Q_OBJECT

		QHash<QString, PrivateChatEntry_ptr> Nick2Entry;
	public:
		PrivateChatManager (QObject*);
		PrivateChatEntry_ptr GetChatEntry (const QString&, IrcServer *, IrcAccount*);
		IrcMessage* CreateMessage (IMessage::MessageType, const QString&, const QString&);
	private:
		PrivateChatEntry_ptr CreateNewChatEntry (const QString&, IrcServer*, IrcAccount*);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_PRIVATECHATMANAGER_H
