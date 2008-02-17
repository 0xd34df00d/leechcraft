#include "socketexceptions.h"

namespace Exceptions
{
    namespace Socket
    {
        BaseSocket::BaseSocket (const QString& reason)
        : Logic (reason.toStdString ())
        {
        }

        BaseSocket::BaseSocket (const std::string& reason)
        : Logic (reason)
        {
        }

        BaseSocket::~BaseSocket () throw ()
        {
        }

        ConnectionRefused::ConnectionRefused (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        RemoteHostClosed::RemoteHostClosed (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        HostNotFound::HostNotFound (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        SocketAccess::SocketAccess (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        SocketResource::SocketResource (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        SocketTimeout::SocketTimeout (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        DatagramTooLarge::DatagramTooLarge (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        Network::Network (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        AddressInUse::AddressInUse (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        SocketAddressNotAvailable::SocketAddressNotAvailable (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        UnsupportedSocketOperation::UnsupportedSocketOperation (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        ProxyAuthenticationRequired::ProxyAuthenticationRequired (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        UnfinishedSocketOperation::UnfinishedSocketOperation (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }

        GenericSocket::GenericSocket (const QString& reason)
        : BaseSocket (reason.toStdString ())
        {
        }
    };
};

