#include <QtDebug>
#include <QThread>
#include <iostream>
#include "tcpsocket.h"
#include "addressparser.h"
#include "proxy.h"
#include "socketexceptions.h"

/*!
 * @brief Default constructor.
 *
 * Creates a TcpSocket with some default DefaultTimeout and null
 * AddressParser.
 *
 * @sa SetDefaultTimeout, GetDefaultTimeout, SetURL, GetAddressParser,
 * AddressParser
 */
TcpSocket::TcpSocket ()
: DefaultTimeout_ (2000)
, AP_ (0)
{
}

/*!
 * @brief Destructor.
 *
 * Destroys a TcpSocket deleting it's AddressParser.
 *
 * @sa AddressParser
 */
TcpSocket::~TcpSocket ()
{
    delete AP_;
}

/*!
 * @brief Connects to a host.
 *
 * Connects to a specified host and blocks until either successful
 * connection or until DefaultTimeout milliseconds would pass, than it
 * throws an exception inherited from Exceptions::Socket::BaseSocket.
 * There are too much kids of it to document in every function.
 *
 * @param[in] host Either IP or human-readable address of server to
 * connect to.
 * @param[in] port Port to connect to.
 *
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 *
 * @sa Disconnect
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 */
void TcpSocket::Connect (const QString& host, int port)
{
    connectToHost (host, port);
    if (state () != ConnectedState && !waitForConnected (DefaultTimeout_))
        ThrowException ();
}

/*!
 * @brief Disconnects from a host.
 *
 * Disconnects from a preciusly connected host or just returns if
 * socket is not connected to any host. It waits for disconnection for
 * DefaultTimeout milliseconds and then throws an exception inherited
 * Exceptions::Socket::BaseSocket if socket hasn't disconnected.
 *
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 * @sa Connect
 */
void TcpSocket::Disconnect ()
{
    if (state () != UnconnectedState)
        disconnectFromHost ();
    if (state () != UnconnectedState && !waitForDisconnected (DefaultTimeout_))
        ThrowException ();
}

/*!
 * @brief Writes string to socket.
 *
 * Overloaded member function, provided for convenience.
 *
 * @param str String to write.
 * @param buffer If true, string is buffered, else Write waits till it
 * is written.
 *
 * @sa Write(const QByteArray&, bool)
 * @sa Flush
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 */
void TcpSocket::Write (const QString& str, bool buffer)
{
    Write (str.toAscii (), buffer);
} 

/*!
 * @brief Writes data to socket.
 *
 * Schedules data for write and returns immidiately if buffer is true,
 * else waits until data is written for DefaultTimeout and in case of
 * timeout, throws an exception of type, derived from
 * Exceptions::Socket::BaseSocket.
 *
 * @param str Data to write.
 * @param buffer If true, data is buffered, else Write waits till it
 * is written.
 *
 * @sa Write(const QString&, bool)
 * @sa Flush
 * @sa GetDefaultTimeout, SetDefaultTimeout
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 */
void TcpSocket::Write (const QByteArray& str, bool buffer)
{
    int result = write (str);
    if (result == -1)
        ThrowException ();
    if (!buffer)
        if (bytesToWrite () && !waitForBytesWritten (DefaultTimeout_))
            ThrowException ();

    qDebug () << "/\\:" << str.trimmed ();
}

/*!
 * @brief Flushes all waiting data.
 *
 * Flushes data scheduled for writing and waits until it is flushed
 * for DefaultTimeout, than in case of timeout throws an exception of
 * type derived from Exceptions::Socket::BaseSocket.
 *
 * @sa Write(const QByteArray&, bool)
 * @sa GetDefaultTimeout, SetDefaultTimeout
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 */
void TcpSocket::Flush ()
{
//  flush ();
    if (bytesToWrite () && !waitForBytesWritten (DefaultTimeout_))
        ThrowException ();
}

/*!
 * @brief Reads a line of data.
 *
 * Waits for DefaultTimoutuntil a line of data is available and
 * returns it. In case of timeout throws an exception derived from
 * Exceptions::Socket::BaseSocket.
 *
 * @return The line of data.
 *
 * @sa GetDefaultTimeout, SetDefaultTimeout, ReadAll
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 */
QByteArray TcpSocket::ReadLine ()
{
    if (canReadLine () || waitForReadyRead (DefaultTimeout_))
    {
        QByteArray response = readLine ();
        qDebug () << "\\/:" << response.trimmed ();
        return response;
    }
    else
        ThrowException ();
}

/*!
 * @brief Reads all available data.
 *
 * If some data is available, returns it all, else waits for
 * DefaultTimeout and returns it then. In case of timeout throws an
 * exception derived from Exceptions::Socket::BaseSocket.
 *
 * @return All available data.
 * @exception Exceptions::Socket::BaseSocket Parent for all socket
 * exceptions.
 */
QByteArray TcpSocket::ReadAll ()
{
    if (bytesAvailable () || waitForReadyRead (DefaultTimeout_))
        return readAll ();
    else
        ThrowException ();
}

/*!
 * @brief Sets default timeout.
 *
 * Sets the timeout for all other operations.
 *
 * @param[in] msecs The timeout value in milliseconds.
 */
void TcpSocket::SetDefaultTimeout (int msecs)
{
    DefaultTimeout_ = msecs;
}

/*!
 * @brief Returns default timeout.
 *
 * Returns the timeout for all other operations.
 *
 * @return The timeout value in milliseconds.
 */
int TcpSocket::GetDefaultTimeout () const
{
    return DefaultTimeout_;
}

/*!
 * @brief Sets the URL for the socket.
 *
 * Sets the URL and parses it.
 *
 * @param[in] str String with the URL.
 * @sa AddressParser
 */
void TcpSocket::SetURL (const QString& str)
{
    delete AP_;
    AP_ = new AddressParser (str);
}

/*!
 * @brief Returns the parser.
 *
 * Returns the address parser for the socket's URL. If not URL was set
 * yet, returns 0.
 *
 * @return The AddressParser object.
 * @sa AddressParser
 */
const AddressParser* TcpSocket::GetAddressParser () const
{
    return AP_;
}

/*!
 * @brief Returns the parser for the URL.
 *
 * Returns the address parser for the passed URL. It's useful if you
 * don't want to create a separate socket only to parse the URL.
 *
 * @param[in] url String with the URL.
 * @return The AddressParser object.
 * @sa AddressParser
 */
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

