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
            BaseSocket (const QString& reason = QString ());
            BaseSocket (const std::string& reason = std::string ());
            virtual ~BaseSocket () throw ();
        };
        struct LEECHCRAFT_API ConnectionRefused : BaseSocket
        {
            ConnectionRefused (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API RemoteHostClosed : BaseSocket
        {
            RemoteHostClosed (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API HostNotFound : BaseSocket
        {
            HostNotFound (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketAccess : BaseSocket
        {
            SocketAccess (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketResource : BaseSocket
        {
            SocketResource (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketTimeout : BaseSocket
        {
            SocketTimeout (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API DatagramTooLarge : BaseSocket
        {
            DatagramTooLarge (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API Network : BaseSocket
        {
            Network (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API AddressInUse : BaseSocket
        {
            AddressInUse (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API SocketAddressNotAvailable : BaseSocket
        {
            SocketAddressNotAvailable (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API UnsupportedSocketOperation : BaseSocket
        {
            UnsupportedSocketOperation (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API ProxyAuthenticationRequired : BaseSocket
        {
            ProxyAuthenticationRequired (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API UnfinishedSocketOperation : BaseSocket
        {
            UnfinishedSocketOperation (const QString& reason = QString ());
        };
        struct LEECHCRAFT_API GenericSocket : BaseSocket
        {
            GenericSocket (const QString& reason = QString ());
        };
    };
};

#endif

