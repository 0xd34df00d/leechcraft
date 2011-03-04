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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H

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
	class IrcServer;
	
	class IrcParser : public QObject
	{
		Q_OBJECT
		IrcServer *IrcServer_;
		ChannelOptions Channel_;
		QString Prefix_;
		QString Command_;
		QStringList Parameters_;
	public:
		IrcParser (IrcServer*);
		void JoinChannel (const ServerOptions&, const ChannelOptions&);
		void NickCommand (const ServerOptions&, const ChannelOptions&);
		void PrivMessageCommand (const QString&, const ServerOptions&, const ChannelOptions&);
		void AuthCommand (const ServerOptions&);
	private:
		void Init ();
		void InitClientConnection (ClientConnection*);
		QString GetPingServer (const QString&) const;
		void PongCommand (const QString&, const QString&);
		void ParseMessage (const QString&);
	public slots:
		void handleServerReply (const QString&, const QString&);
	private slots:
		void SuccessfulAuth ();
		void UserCommand (const ServerOptions&);
	signals:
		void readyToReadAnswer (const QString&, const QString&);
		void gotAuthSuccess ();
		void gotCLEntries (const QString&, const QString&);
		void gotTopic (const QString&, const QString&);
		void messageReceived (const QString&, const QString&, const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H
