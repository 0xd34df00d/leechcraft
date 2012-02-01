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

#include <QBuffer>
#include <QTimer>
#include "FakeSocket.h"

namespace Imap {

FakeSocket::FakeSocket()
{
    readChannel = new QBuffer( &r, this );
    readChannel->open( QIODevice::ReadWrite );
    writeChannel = new QBuffer( &w, this );
    writeChannel->open( QIODevice::WriteOnly );
    QTimer::singleShot( 0, this, SLOT(slotEmitConnected()) );
    connect( readChannel, SIGNAL(readyRead()), this, SIGNAL(readyRead()) );
}

FakeSocket::~FakeSocket()
{
}

void FakeSocket::slotEmitConnected()
{
    // We have to use both conventions for letting the world know that "we're finally usable"
    emit connected();
    emit stateChanged(Imap::CONN_STATE_ESTABLISHED, QString());
}

void FakeSocket::fakeReading( const QByteArray& what )
{
    // The position of the cursor is shared for both reading and wirting, and therefore
    // we have to save and restore it after appending data, otherwise the pointer will
    // be left scrolled to after the actual data, failing further attempts to read the
    // data back. It's pretty obvious when you think about it, but took sime time to
    // debug nevertheless :).
    qint64 pos = readChannel->pos();
    readChannel->write( what );
    readChannel->seek( pos );
}

bool FakeSocket::canReadLine()
{
    return readChannel->canReadLine();
}

QByteArray FakeSocket::read( qint64 maxSize )
{
    return readChannel->read( maxSize );
}

QByteArray FakeSocket::readLine( qint64 maxSize )
{
    return readChannel->readLine( maxSize );
}

qint64 FakeSocket::write( const QByteArray& byteArray )
{
    return writeChannel->write( byteArray );
}

void FakeSocket::startTls()
{
    // fake it
    writeChannel->write( QByteArray("[*** STARTTLS ***]") );
}

bool FakeSocket::isDead()
{
    // Can't really die (yet)
    return false;
}

QByteArray FakeSocket::writtenStuff()
{
    QByteArray res = w;
    w.clear();
    writeChannel->seek( 0 );
    return res;
}

}
