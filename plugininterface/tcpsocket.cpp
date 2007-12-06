#include <QtDebug>
#include <QThread>
#include <iostream>
#include "tcpsocket.h"
#include "addressparser.h"
#include "proxy.h"
#include "socketexceptions.h"

TcpSocket::TcpSocket ()
: DefaultTimeout_ (2000)
, AP_ (0)
{
}

TcpSocket::~TcpSocket ()
{
	delete AP_;
}

void TcpSocket::Connect (const QString& host, int port)
{
	connectToHost (host, port);
	if (!waitForConnected (DefaultTimeout_))
		ThrowException ();
}

void TcpSocket::Disconnect ()
{
	if (state () != UnconnectedState)
		disconnectFromHost ();
	if (state () != UnconnectedState && !waitForDisconnected (DefaultTimeout_))
		ThrowException ();
}

void TcpSocket::Write (const QString& str, bool buffer)
{
	Write (str.toAscii (), buffer);
	Proxy::Instance ()->AddUploadMessage (str.trimmed () + QString ("") + (buffer ? QString (tr ("[buffered]")) : QString (tr ("[unbuffered]"))));
} 

void TcpSocket::Write (const QByteArray& str, bool buffer)
{
	int result = write (str);
	if (result == -1)
		ThrowException ();
	if (!buffer)
		if (!waitForBytesWritten (DefaultTimeout_))
			ThrowException ();

	qDebug () << "/\\:" << str.trimmed ();
}

void TcpSocket::Flush ()
{
//	flush ();
	if (!waitForBytesWritten (DefaultTimeout_))
		ThrowException ();
}

QByteArray TcpSocket::ReadLine ()
{
	if (canReadLine () || waitForReadyRead (DefaultTimeout_))
	{
		QByteArray response = readLine ();
		qDebug () << "\\/:" << response.trimmed ();
		Proxy::Instance ()->AddDownloadMessage (response.trimmed ());
		return response;
	}
	else
		ThrowException ();
}

QByteArray TcpSocket::ReadAll ()
{
	if (bytesAvailable () || waitForReadyRead (DefaultTimeout_))
		return readAll ();
	else
		ThrowException ();
}

void TcpSocket::SetDefaultTimeout (int msecs)
{
	DefaultTimeout_ = msecs;
}

int TcpSocket::GetDefaultTimeout () const
{
	return DefaultTimeout_;
}

void TcpSocket::SetURL (const QString& str)
{
	delete AP_;
	AP_ = new AddressParser (str);
}

const AddressParser* TcpSocket::GetAddressParser () const
{
	return AP_;
}

AddressParser* TcpSocket::GetAddressParser (const QString& url)
{
	return new AddressParser (url);
}

void TcpSocket::ThrowException () const
{
	switch (error ())
	{
		case ConnectionRefusedError:
			throw Exceptions::Socket::ConnectionRefused (errorString ());
		case RemoteHostClosedError:
			throw Exceptions::Socket::RemoteHostClosed (errorString ());
		case HostNotFoundError:
			throw Exceptions::Socket::HostNotFound (errorString ());
		case SocketAccessError:
			throw Exceptions::Socket::SocketAccess (errorString ());
		case SocketResourceError:
			throw Exceptions::Socket::SocketResource (errorString ());
		case SocketTimeoutError:
			throw Exceptions::Socket::SocketTimeout (errorString ());
		case DatagramTooLargeError:
			throw Exceptions::Socket::DatagramTooLarge (errorString ());
		case NetworkError:
			throw Exceptions::Socket::Network (errorString ());
		case AddressInUseError:
			throw Exceptions::Socket::AddressInUse (errorString ());
		case SocketAddressNotAvailableError:
			throw Exceptions::Socket::SocketAddressNotAvailable (errorString ());
		case UnsupportedSocketOperationError:
			throw Exceptions::Socket::UnsupportedSocketOperation (errorString ());
		case ProxyAuthenticationRequiredError:
			throw Exceptions::Socket::ProxyAuthenticationRequired (errorString ());
		case UnfinishedSocketOperationError:
			throw Exceptions::Socket::UnfinishedSocketOperation (errorString ());
		case UnknownSocketError:
			throw Exceptions::Socket::GenericSocket (errorString ());
	}
}

