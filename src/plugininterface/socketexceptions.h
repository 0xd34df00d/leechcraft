#ifndef SOCKETEXCEPTIONS_H
#define SOCKETEXCEPTIONS_H
#include <exceptions/logic.h>
#include <QString>
#include "config.h"

namespace Exceptions
{
    namespace Socket
    {
        struct LEECHCRAFT_API BaseSocket : Logic
        {
            LEECHCRAFT_API BaseSocket (const QString& reason = QString ());
            LEECHCRAFT_API BaseSocket (const std::string& reason = std::string ());
            LEECHCRAFT_API virtual ~BaseSocket () throw ();
        };
        struct LEECHCRAFT_API ConnectionRefused : BaseSocket
        {
            LEECHCRAFT_API ConnectionRefused (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API RemoteHostClosed : BaseSocket
        {
            LEECHCRAFT_API RemoteHostClosed (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API HostNotFound : BaseSocket
        {
            LEECHCRAFT_API HostNotFound (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketAccess : BaseSocket
        {
            LEECHCRAFT_API SocketAccess (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketResource : BaseSocket
        {
            LEECHCRAFT_API SocketResource (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketTimeout : BaseSocket
        {
            LEECHCRAFT_API SocketTimeout (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API DatagramTooLarge : BaseSocket
        {
            LEECHCRAFT_API DatagramTooLarge (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API Network : BaseSocket
        {
            LEECHCRAFT_API Network (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API AddressInUse : BaseSocket
        {
            LEECHCRAFT_API AddressInUse (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketAddressNotAvailable : BaseSocket
        {
            LEECHCRAFT_API SocketAddressNotAvailable (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API UnsupportedSocketOperation : BaseSocket
        {
            LEECHCRAFT_API UnsupportedSocketOperation (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API ProxyAuthenticationRequired : BaseSocket
        {
            LEECHCRAFT_API ProxyAuthenticationRequired (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API UnfinishedSocketOperation : BaseSocket
        {
            LEECHCRAFT_API UnfinishedSocketOperation (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API GenericSocket : BaseSocket
        {
            LEECHCRAFT_API GenericSocket (const QString& reason = QString ());
        };
    };
};

#endif

