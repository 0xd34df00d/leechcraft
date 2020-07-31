/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "balancer.h"
#include <QTcpSocket>
#include <QtDebug>

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	Balancer::Balancer (QObject *parent)
	: QObject (parent)
	{
	}

	void Balancer::GetServer ()
	{
		QTcpSocket *socket = new QTcpSocket (this);
		socket->connectToHost ("mrim.mail.ru", 443);
		connect (socket,
				SIGNAL (readyRead ()),
				this,
				SLOT (handleRead ()));
		connect (socket,
				SIGNAL (error (QAbstractSocket::SocketError)),
				this,
				SLOT (handleSocketError (QAbstractSocket::SocketError)));
	}

	void Balancer::handleRead ()
	{
		QTcpSocket *socket = qobject_cast<QTcpSocket*> (sender ());
		if (!socket->canReadLine ())
		{
			qWarning () << Q_FUNC_INFO
					<< "can't read line from socket, waiting more...";
			return;
		}

		socket->deleteLater ();

		const QByteArray& line = socket->readAll ().trimmed ();
		const int pos = line.indexOf (':');
		if (pos <= 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "got"
					<< line;
			emit error ();
			return;
		}

		const QString& server = line.left (pos);
		const int port = line.mid (pos + 1).toInt ();

		if (port <= 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid port"
					<< server
					<< port
					<< line
					<< line.mid (pos + 1);
			emit error ();
			return;
		}

		emit gotServer (server, port);
		disconnect (socket,
				0,
				this,
				0);
	}

	void Balancer::handleSocketError (QAbstractSocket::SocketError code)
	{
		QTcpSocket *socket = qobject_cast<QTcpSocket*> (sender ());
		qWarning () << Q_FUNC_INFO
				<< "socket error"
				<< code
				<< socket->errorString ();
		socket->deleteLater ();
		emit error ();
	}
}
}
}
}
