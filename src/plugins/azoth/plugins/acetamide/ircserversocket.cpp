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
#include <QSslSocket>
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
	, SSL_ (ish->GetServerOptions ().SSL_)
	{
		Socket_ptr.reset (SSL_ ? new QSslSocket : new QTcpSocket);
		Init ();
	}

	void IrcServerSocket::ConnectToHost (const QString& host, int port)
	{
		if (!SSL_)
			Socket_ptr->connectToHost (host, port);
		else
		{
			std::shared_ptr<QSslSocket> s = std::dynamic_pointer_cast<QSslSocket> (Socket_ptr);
			s->connectToHostEncrypted (host, port);
		}
	}

	void IrcServerSocket::DisconnectFromHost ()
	{
		Socket_ptr->disconnectFromHost ();
	}

	void IrcServerSocket::Send (const QString& message)
	{
		if (!Socket_ptr->isWritable ())
		{
			qWarning () << Q_FUNC_INFO
					<< Socket_ptr->error ()
					<< Socket_ptr->errorString ();
			return;
		}

		if (Socket_ptr->write (message.toAscii ()) == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< Socket_ptr->error ()
					<< Socket_ptr->errorString ();
			return;
		}
	}

	void IrcServerSocket::Close ()
	{
		Socket_ptr->close ();
	}

	void IrcServerSocket::Init ()
	{
		connect (Socket_ptr.get (),
				SIGNAL (readyRead ()),
				this,
				SLOT (readReply ()));

		connect (Socket_ptr.get (),
				SIGNAL (connected ()),
				ISH_,
				SLOT (connectionEstablished ()));

		connect (Socket_ptr.get (),
				SIGNAL (disconnected ()),
				ISH_,
				SLOT (connectionClosed ()));

		connect (Socket_ptr.get (),
				SIGNAL (error (QAbstractSocket::SocketError)),
				Account_->GetClientConnection ().get (),
				SLOT (handleError (QAbstractSocket::SocketError)));
	}

	void IrcServerSocket::readReply ()
	{
		while (Socket_ptr->canReadLine ())
			ISH_->ReadReply (Socket_ptr->readLine ());
	}

};
};
};
