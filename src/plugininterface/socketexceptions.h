#ifndef SOCKETEXCEPTIONS_H
#define SOCKETEXCEPTIONS_H
#include <exceptions/logic.h>
#include <QString>

namespace Exceptions
{
    namespace Socket
    {
        struct BaseSocket : Logic
        {
            BaseSocket (const QString& reason = QString ());
            BaseSocket (const std::string& reason = std::string ());
            virtual ~BaseSocket () throw ();
        };
        struct ConnectionRefused : BaseSocket
        {
            ConnectionRefused (const QString& reason = QString ());
        };
        struct RemoteHostClosed : BaseSocket
        {
            RemoteHostClosed (const QString& reason = QString ());
        };
        struct HostNotFound : BaseSocket
        {
            HostNotFound (const QString& reason = QString ());
        };
        struct SocketAccess : BaseSocket
        {
            SocketAccess (const QString& reason = QString ());
        };
        struct SocketResource : BaseSocket
        {
            SocketResource (const QString& reason = QString ());
        };
        struct SocketTimeout : BaseSocket
        {
            SocketTimeout (const QString& reason = QString ());
        };
        struct DatagramTooLarge : BaseSocket
        {
            DatagramTooLarge (const QString& reason = QString ());
        };
        struct Network : BaseSocket
        {
            Network (const QString& reason = QString ());
        };
        struct AddressInUse : BaseSocket
        {
            AddressInUse (const QString& reason = QString ());
        };
        struct SocketAddressNotAvailable : BaseSocket
        {
            SocketAddressNotAvailable (const QString& reason = QString ());
        };
        struct UnsupportedSocketOperation : BaseSocket
        {
            UnsupportedSocketOperation (const QString& reason = QString ());
        };
        struct ProxyAuthenticationRequired : BaseSocket
        {
            ProxyAuthenticationRequired (const QString& reason = QString ());
        };
        struct UnfinishedSocketOperation : BaseSocket
        {
            UnfinishedSocketOperation (const QString& reason = QString ());
        };
        struct GenericSocket : BaseSocket
        {
            GenericSocket (const QString& reason = QString ());
        };
    };
};

#endif

