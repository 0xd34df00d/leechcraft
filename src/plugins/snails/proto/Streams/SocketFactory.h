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

#ifndef IMAP_SOCKETFACTORY_H
#define IMAP_SOCKETFACTORY_H

#include <QPointer>
#include <QStringList>
#include "Socket.h"

/** @short Namespace for IMAP interaction */
namespace Imap {

/** @short Classes for handling of mailboxes and connections */
namespace Mailbox {

/** @short Abstract interface for creating new socket that is somehow connected
 * to the IMAP server */
class SocketFactory: public QObject {
    Q_OBJECT
    bool _startTls;
public:
    SocketFactory();
    virtual ~SocketFactory() {};
    /** @short Create new socket and return a smart pointer to it */
    virtual Socket* create() = 0;
    void setStartTlsRequired( const bool doIt );
    bool startTlsRequired();
signals:
    void error( const QString& );
};

typedef std::shared_ptr<SocketFactory> SocketFactoryPtr;

/** @short Manufacture sockets based on QProcess */
class ProcessSocketFactory: public SocketFactory {
    Q_OBJECT
    /** @short Name of executable file to launch */
    QString _executable;
    /** @short Arguments to launch the process with */
    QStringList _args;
public:
    ProcessSocketFactory( const QString& executable, const QStringList& args );
    virtual Socket* create();
};

/** @short Manufacture sockets based on QSslSocket */
class SslSocketFactory: public SocketFactory {
    Q_OBJECT
    /** @short Hostname of the remote host */
    QString _host;
    /** @short Port number */
    quint16 _port;
public:
    SslSocketFactory( const QString& host, const quint16 port );
    virtual Socket* create();
};

/** @short Factory for regular TCP sockets that are able to STARTTLS */
class TlsAbleSocketFactory: public SocketFactory {
    Q_OBJECT
    /** @short Hostname of the remote host */
    QString _host;
    /** @short Port number */
    quint16 _port;
public:
    TlsAbleSocketFactory( const QString& host, const quint16 port );
    virtual Socket* create();
};

/** @short A fake factory suitable for unit tests */
class FakeSocketFactory: public SocketFactory {
    Q_OBJECT
public:
    FakeSocketFactory();
    virtual Socket* create();
    /** @short Return the last created socket */
    Socket* lastSocket();
private:
    QPointer<Socket> _last;
};


}

}

#endif /* IMAP_SOCKETFACTORY_H */
