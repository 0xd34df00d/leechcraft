/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "socketerrorstrings.h"
#include <QObject>

namespace LC
{
namespace Util
{
	QString GetSocketErrorString (QAbstractSocket::SocketError error)
	{
		switch (error)
		{
		case QAbstractSocket::ConnectionRefusedError:
			return QObject::tr ("connection refused");	
		case QAbstractSocket::RemoteHostClosedError:
			return QObject::tr ("remote host closed connection");
		case QAbstractSocket::HostNotFoundError:
			return QObject::tr ("host not found");
		case QAbstractSocket::SocketAccessError:
			return QObject::tr ("socket access error (lacking required privileges)");
		case QAbstractSocket::SocketResourceError:
			return QObject::tr ("system ran out of sockets");
		case QAbstractSocket::SocketTimeoutError:
			return QObject::tr ("socket operation timed out");
		case QAbstractSocket::DatagramTooLargeError:
			return QObject::tr ("datagram too large");
		case QAbstractSocket::NetworkError:
			return QObject::tr ("physical network error");
		case QAbstractSocket::AddressInUseError:
			return QObject::tr ("address already in use");
		case QAbstractSocket::SocketAddressNotAvailableError:
			return QObject::tr ("specified address doesn't belong to the host");
		case QAbstractSocket::UnsupportedSocketOperationError:
			return QObject::tr ("unsupported socket operation");
		case QAbstractSocket::ProxyAuthenticationRequiredError:
			return QObject::tr ("proxy authentication required");
		case QAbstractSocket::SslHandshakeFailedError:
			return QObject::tr ("SSL handshake failed");
		case QAbstractSocket::UnfinishedSocketOperationError:
			return QObject::tr ("unfinished socket operation in progress");
		case QAbstractSocket::ProxyConnectionRefusedError:
			return QObject::tr ("proxy connection has been refused");
		case QAbstractSocket::ProxyConnectionClosedError:
			return QObject::tr ("proxy connection was closed unexpectedly");
		case QAbstractSocket::ProxyConnectionTimeoutError:
			return QObject::tr ("connection to the proxy server timed out");
		case QAbstractSocket::ProxyNotFoundError:
			return QObject::tr ("proxy not found");
		case QAbstractSocket::ProxyProtocolError:
			return QObject::tr ("proxy protocol error");
		case QAbstractSocket::UnknownSocketError:
		default:
			return QObject::tr ("unknown error");
		}
	}
}
}
