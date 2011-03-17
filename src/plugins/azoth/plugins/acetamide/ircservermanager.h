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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERMANAGER_H

#include <QObject>
#include <QMap>
#include "ircserver.h"
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;
	class IrcMessage;
	class ClientConnection;

	class IrcServerManager : public QObject
	{
		Q_OBJECT
		QMap<IrcAccount*, IrcServer_ptr> Account2Server;
	public:
		IrcServerManager (QObject*);
		void JoinChannel (const ServerOptions&, const ChannelOptions&, IrcAccount*);
		void SetTopic (const QString&, const QString&, const QString&);
		void SetCLEntries (const QString&, const QString&, const QString&);
		void SetMessageIn (const QString&, const QString&, const QString&, const QString&);
		void SetMessageOut (const QString&, const ChannelOptions&, IrcAccount*);
		void SetPrivateMessageOut (IrcAccount*, IrcMessage*);
		void LeaveChannel (const QString&, IrcAccount*);
		void SetNewParticipant (const QString&, const QString&, const QString&);
		void SetUserLeave (const QString&, const QString&, const QString&, const QString&);
		QList<IrcAccount*> GetAccounts (IrcServer*) const;
		IrcServer_ptr GetServer (const QString&) const;
	public slots:
		void changeState (const QString&, ConnectionState);
		void handleAnswer (const QString&, const QString&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERMANAGER_H
