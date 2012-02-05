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
#ifndef IMAP_IODEVICE_SOCKET_H
#define IMAP_IODEVICE_SOCKET_H

#include <QAbstractSocket>
#include <QProcess>
#include "Socket.h"

class QSslSocket;
class QTimer;

namespace Imap {

namespace Mailbox {
class SocketFactory;
}

    /** @short Helper class for all sockets which are based on a QIODevice */
    class IODeviceSocket: public Socket {
        Q_OBJECT
        Q_DISABLE_COPY(IODeviceSocket)
    public:
        IODeviceSocket( QIODevice* device);
        ~IODeviceSocket();
        virtual bool canReadLine();
        virtual QByteArray read( qint64 maxSize );
        virtual QByteArray readLine( qint64 maxSize = 0 );
        virtual qint64 write( const QByteArray& byteArray );
        virtual void startTls();
        virtual bool isDead() = 0;
    private slots:
        virtual void handleStateChanged() = 0;
        virtual void delayedStart() = 0;
        void emitError();
    protected:
        QIODevice* d;
        QTimer* delayedDisconnect;
        QString disconnectedMessage;
    };

    /** @short A QProcess-based socket */
    class ProcessSocket: public IODeviceSocket {
        Q_OBJECT
        Q_DISABLE_COPY(ProcessSocket);
    public:
        ProcessSocket(QProcess *proc, const QString& executable, const QStringList& args);
        ~ProcessSocket();
        bool isDead();
    private slots:
        void handleStateChanged();
        void handleProcessError( QProcess::ProcessError );
        void delayedStart();
    private:
        QString _executable;
        QStringList _args;
    };

    /** @short An SSL socket, usable both in SSL-from-start and STARTTLS-on-demand mode */
    class SslTlsSocket: public IODeviceSocket {
        Q_OBJECT
        Q_DISABLE_COPY(SslTlsSocket);
    public:
        /** Set the @arg startEncrypted to true if the wrapper is supposed to emit
        connected() only after it has established proper encryption */
        SslTlsSocket(QSslSocket *sock, const QString& host, const quint16 port, const bool startEncrypted=false);
        bool isDead();
    private slots:
        void handleStateChanged();
        void handleSocketError( QAbstractSocket::SocketError );
        void delayedStart();
    private:
        bool _startEncrypted;
        QString _host;
        quint16 _port;
    };

};

#endif /* IMAP_IODEVICE_SOCKET_H */
