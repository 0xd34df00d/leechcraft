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

#ifndef IMAP_FAKE_SOCKET_H
#define IMAP_FAKE_SOCKET_H

#include <QAbstractSocket>
#include <QProcess>
#include "Socket.h"

class QTimer;

namespace Imap {

    /** @short A fake socket implementation, useful for automated unit tests

Typical use:

    model->rowCount( QModelIndex() );
    SOCK->fakeReading( "* PREAUTH foo\r\n" );
    QTest::qWait( 100 );
    QCOMPARE( SOCK->writtenStuff(), QByteArray("y1 CAPABILITY\r\ny0 LIST \"\" \"%\"\r\n") );
    SOCK->fakeReading( "* LIST (\\HasNoChildren) \".\" \"INBOX\"\r\n"
                       "* CAPABILITY IMAP4rev1\r\n"
                       "y1 OK capability completed\r\n"
                       "y0 ok list completed\r\n" );
    QTest::qWait( 100 );
    QModelIndex inbox = model->index( 1, 0, QModelIndex() );
    QCOMPARE( model->data( inbox, Qt::DisplayRole ), QVariant("INBOX") );
*/
    class FakeSocket: public Socket {
        Q_OBJECT
    public:
        FakeSocket();
        ~FakeSocket();
        virtual bool canReadLine();
        virtual QByteArray read( qint64 maxSize );
        virtual QByteArray readLine( qint64 maxSize = 0 );
        virtual qint64 write( const QByteArray& byteArray );
        virtual void startTls();
        virtual bool isDead();

        /** @short Return data written since the last call to this function */
        QByteArray writtenStuff();

    private slots:
        /** @short Delayed informing about being connected */
        void slotEmitConnected();

    public slots:
        /** @short Simulate arrival of some data

The provided @arg what data are appended to the internal buffer and relevant signals
are emitted. This function currently does not free the occupied memory, which might
eventually lead to certain troubles.
*/
        void fakeReading( const QByteArray& what );

    private:
        QIODevice* readChannel;
        QIODevice* writeChannel;

        QByteArray r, w;

        FakeSocket(const FakeSocket&); // don't implement
        FakeSocket& operator=(const FakeSocket&); // don't implement
    };

};

#endif /* IMAP_FAKE_SOCKET_H */
