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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QTcpSocket>
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;
	class IrcParser;
	class IrcServerCLEntry;

	class IrcServerHandler : public QObject
	{
		Q_OBJECT

		IrcAccount *Account_;
		IrcParser *IrcParser_;
		IrcServerCLEntry *ServerCLEntry_;
		ServerOptions ServerOptions_;
		QString ServerID_;
		boost::shared_ptr<QTcpSocket> TcpSocket_ptr;
		ConnectionState ServerConnectionState_;
		QList<ChannelOptions> ActiveChannels_;
	public:
		IrcServerHandler (const ServerOptions&, IrcAccount*);
		IrcServerCLEntry* GetCLEntry () const;
		IrcAccount* GetAccount () const;

		QString GetServerID_ () const;
		ServerOptions GetServerOptions () const;
		ConnectionState GetConnectionState () const;

		bool ConnectToServer ();
		void SendCommand (const QString&);
	private:
		void InitSocket ();
	private slots:
		void readReply ();
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERHANDLER_H
