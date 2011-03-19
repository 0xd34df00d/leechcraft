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

#include <boost/function.hpp>
#include <QObject>
#include <QMap>
#include <QHash>
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

		QMap<IrcAccount*, QHash<QString, IrcServer_ptr> > Account2Server_;
		QHash <QString, IrcServer_ptr> Key2Server_;
	public:
		IrcServerManager (QObject*);
		QSet<IrcServer_ptr> GetServers (IrcAccount*);
		QSet<IrcAccount*> GetAccounts (const QString&);
		void JoinChannel (const ServerOptions&, const ChannelOptions&, IrcAccount*);
		void SetTopic (const QString&, const QString&, const QString&);
		void SetCLEntries (const QString&, const QString&, const QString&);
		void SetMessageIn (const QString&, const QString&, const QString&, const QString&);
		void SetMessageOut (const QString&, const ChannelOptions&, const ServerOptions&, IrcAccount*);
		void SetPrivateMessageOut (IrcAccount*, IrcMessage*);
		void LeaveChannel (const QString&, const QString&, IrcAccount*);
		void SetNewParticipant (const QString&, const QString&, const QString&);
		void SetUserLeave (const QString&, const QString&, const QString&, const QString&);
		QList<IrcAccount*> GetAccounts (IrcServer*) const;
		void RemoveServer (const QString&);
		bool DoServerAction (boost::function<void (IrcServer_ptr)>, const QString&);
		bool DoClientConnectionAction (boost::function<void (ClientConnection*)>, const QString&);
		bool ServerExists (QHash<QString, IrcServer_ptr>, const QString&);
	public slots:
		void changeState (const QString&, ConnectionState);
		void handleAnswer (const QString&, const QString&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERMANAGER_H
