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

#include "ircserverhandler.h"
#include "ircaccount.h"
#include "ircparser.h"
#include "ircserverclentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerHandler::IrcServerHandler (const ServerOptions& server,
			IrcAccount *account )
	: Account_ (account)
	, ServerOptions_ (server)
	, ServerCLEntry_ (new IrcServerCLEntry (this, account))
	, ServerID_ (server.ServerName_ + ":" + 
			QString::number (server.ServerPort_))
	, ServerConnectionState_ (NotConnected)
	{
		IrcParser_ = new IrcParser (this);
	}

	IrcServerCLEntry* IrcServerHandler::GetCLEntry () const
	{
		return ServerCLEntry_;
	}

	IrcAccount* IrcServerHandler::GetAccount () const
	{
		return Account_;
	}

	QString IrcServerHandler::GetServerID_ () const
	{
		return ServerID_;
	}

	ServerOptions IrcServerHandler::GetServerOptions () const
	{
		return ServerOptions_;
	}

	ConnectionState IrcServerHandler::GetConnectionState () const
	{
		return ServerConnectionState_;
	}

	bool IrcServerHandler::ConnectToServer ()
	{
		if (ServerConnectionState_ == NotConnected)
		{
			TcpSocket_ptr.reset (new QTcpSocket (this));
			ServerConnectionState_ = InProcess;

			TcpSocket_ptr->connectToHost (ServerOptions_.ServerName_,
					ServerOptions_.ServerPort_);

			if (!TcpSocket_ptr->waitForConnected(30000))
			{
				ServerConnectionState_ = NotConnected;
				qDebug () << Q_FUNC_INFO
						<< "cannot to connect to host"
						<< ServerID_;
				return false; 
			}
			
			ServerConnectionState_ = Connected;
			ServerCLEntry_->
					SetStatus (EntryStatus (SOnline, QString ()));
			InitSocket ();
			IrcParser_->AuthCommand ();
		}
	}

	void IrcServerHandler::SendCommand (const QString& cmd)
	{
		qDebug () << TcpSocket_ptr.get () << cmd;
		if (!TcpSocket_ptr->isWritable ())
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}
			
		if (TcpSocket_ptr->write (cmd.toAscii ()) == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}
	}

	void IrcServerHandler::InitSocket ()
	{
		connect (TcpSocket_ptr.get (),
				SIGNAL (readyRead ()),
				this,
				SLOT (readReply ()));
	}

	void IrcServerHandler::readReply ()
	{
		while (TcpSocket_ptr->canReadLine ())
		{
			QString str = TcpSocket_ptr->readLine ();
			qDebug () << str;
		}
	}

};
};
};