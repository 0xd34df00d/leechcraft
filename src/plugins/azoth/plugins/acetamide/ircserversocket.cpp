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

#include "ircserversocket.h"
#include <QTcpSocket>
#include "ircserverhandler.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerSocket::IrcServerSocket (IrcServerHandler *ish)
	: QObject (ish)
	, Account_ (ish->GetAccount ())
	, ISH_ (ish)
	{
		TcpSocket_ptr.reset (new QTcpSocket);
		Init ();
	}

	void IrcServerSocket::ConnectToHost (const QString& host, int port)
	{
		TcpSocket_ptr->connectToHost (host, port);
	}

	void IrcServerSocket::DisconnectFromHost ()
	{
		TcpSocket_ptr->disconnectFromHost ();
	}

	void IrcServerSocket::Send (const QString& message)
	{
		if (!TcpSocket_ptr->isWritable ())
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}

		if (TcpSocket_ptr->write (message.toAscii ()) == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}
	}

	void IrcServerSocket::Close ()
	{
		TcpSocket_ptr->close ();
	}

	void IrcServerSocket::Init ()
	{
		connect (TcpSocket_ptr.get (),
				SIGNAL (readyRead ()),
				this,
				SLOT (readReply ()));

		connect (TcpSocket_ptr.get (),
				SIGNAL (connected ()),
				ISH_,
				SLOT (connectionEstablished ()));

		connect (TcpSocket_ptr.get (),
				SIGNAL (disconnected ()),
				ISH_,
				SLOT (connectionClosed ()));

		connect (TcpSocket_ptr.get (),
				SIGNAL (error (QAbstractSocket::SocketError)),
				Account_->GetClientConnection ().get (),
				SLOT (handleError (QAbstractSocket::SocketError)));
	}

	void IrcServerSocket::readReply ()
	{
		while (TcpSocket_ptr->canReadLine ())
			ISH_->ReadReply (TcpSocket_ptr->readLine ());
	}

};
};
};