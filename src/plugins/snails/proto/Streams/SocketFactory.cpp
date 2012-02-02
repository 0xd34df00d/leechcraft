/* Copyright (C) 2006 - 2011 Jan Kundrát <jkt@gentoo.org>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or the version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QProcess>
#include <QSslSocket>
#include "SocketFactory.h"
#include "IODeviceSocket.h"
#include "FakeSocket.h"

namespace Imap {
namespace Mailbox {

SocketFactory::SocketFactory(): _startTls(false)
{
}

void SocketFactory::setStartTlsRequired( const bool doIt )
{
    _startTls = doIt;
}

bool SocketFactory::startTlsRequired()
{
    return _startTls;
}

ProcessSocketFactory::ProcessSocketFactory(
        const QString& executable, const QStringList& args):
    _executable(executable), _args(args)
{
}

Socket* ProcessSocketFactory::create()
{
    // FIXME: this may leak memory if an exception strikes in this function
    // (before we return the pointer)
    return new ProcessSocket(new QProcess(), _executable, _args);
}

SslSocketFactory::SslSocketFactory( const QString& host, const quint16 port ):
    _host(host), _port(port)
{
}

Socket* SslSocketFactory::create()
{
    QSslSocket* sslSock = new QSslSocket();
    IODeviceSocket* ioSock = new SslTlsSocket( sslSock, _host, _port, true );
    return ioSock;
}

TlsAbleSocketFactory::TlsAbleSocketFactory( const QString& host, const quint16 port ):
    _host(host), _port(port)
{
}

Socket* TlsAbleSocketFactory::create()
{
    QSslSocket* sslSock = new QSslSocket();
    return new SslTlsSocket(sslSock, _host, _port);
}

FakeSocketFactory::FakeSocketFactory(): SocketFactory()
{
}

Socket* FakeSocketFactory::create()
{
    return _last = new FakeSocket();
}

Socket* FakeSocketFactory::lastSocket()
{
    Q_ASSERT(_last);
    return _last;
}



}
}
