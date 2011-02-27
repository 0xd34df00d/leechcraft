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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCCLIENT_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCCLIENT_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include "core.h"
#include "socketmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccount;
	class ClientConnection;
	class IrcClient : public QObject
	{
		Q_OBJECT
		int incr;
		boost::shared_ptr<SocketManager> SocketManager_;
		QHash<QString, QString> Nicknames_;
		QHash<QString, QString> Topic_;
		QMap <QString, QRegExp> Name2RegExp_;
	public:
		IrcClient (QObject*);
		void JoinChannel (const ServerOptions&, const ChannelOptions&, ClientConnection*);
		void NickCommand (const ServerOptions&, const ChannelOptions&);
		void PrivMessageCommand (const QString&, const ServerOptions&, const ChannelOptions&);
	private:
		void Init ();
		void InitClientConnection (ClientConnection*);
		void AuthCommand (const ServerOptions&, const ChannelOptions&);
		QString GetPingServer (const QString&) const;
		void PongCommand (const QString&, const QString&);
	public slots:
		void handleServerReply (const QString&, const QString&);
	signals:
		void readyToReadAnswer (const QString&, const QString&);
		void gotCLEntries (const QString&, const QString&);
		void gotTopic (const QString&, const QString&);
		void messageReceived (const QString&, const QString&, const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCCLIENT_H
