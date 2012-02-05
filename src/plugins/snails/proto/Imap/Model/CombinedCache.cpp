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

#include "CombinedCache.h"
#include "DiskPartCache.h"
#include "SQLCache.h"

namespace Imap {
namespace Mailbox {

CombinedCache::CombinedCache( QObject* parent, const QString& name, const QString& cacheDir ):
        AbstractCache(parent), _name(name), _cacheDir(cacheDir)
{
    _sqlCache = new SQLCache( this );
    connect( _sqlCache, SIGNAL(error(QString)), this, SIGNAL(error(QString)) );
    _diskPartCache = new DiskPartCache( this, cacheDir );
    connect( _diskPartCache, SIGNAL(error(QString)), this, SIGNAL(error(QString)) );
}

CombinedCache::~CombinedCache()
{
}

bool CombinedCache::open()
{
    return _sqlCache->open( _name, _cacheDir + QLatin1String("/imap.cache.sqlite") );
}

QList<MailboxMetadata> CombinedCache::childMailboxes( const QString& mailbox ) const
{
    return _sqlCache->childMailboxes( mailbox );
}

bool CombinedCache::childMailboxesFresh( const QString& mailbox ) const
{
    return _sqlCache->childMailboxesFresh( mailbox );
}

void CombinedCache::setChildMailboxes( const QString& mailbox, const QList<MailboxMetadata>& data )
{
    _sqlCache->setChildMailboxes( mailbox, data );
}

void CombinedCache::forgetChildMailboxes( const QString& mailbox )
{
    _sqlCache->forgetChildMailboxes( mailbox );
}

SyncState CombinedCache::mailboxSyncState( const QString& mailbox ) const
{
    return _sqlCache->mailboxSyncState( mailbox );
}

void CombinedCache::setMailboxSyncState( const QString& mailbox, const SyncState& state )
{
    _sqlCache->setMailboxSyncState( mailbox, state );
}

QList<uint> CombinedCache::uidMapping( const QString& mailbox ) const
{
    return _sqlCache->uidMapping( mailbox );
}

void CombinedCache::setUidMapping( const QString& mailbox, const QList<uint>& seqToUid )
{
    _sqlCache->setUidMapping( mailbox, seqToUid );
}

void CombinedCache::clearUidMapping( const QString& mailbox )
{
    _sqlCache->clearUidMapping( mailbox );
}

void CombinedCache::clearAllMessages( const QString& mailbox )
{
    _sqlCache->clearAllMessages( mailbox );
    _diskPartCache->clearAllMessages( mailbox );
}

void CombinedCache::clearMessage( const QString mailbox, uint uid )
{
    _sqlCache->clearMessage( mailbox, uid );
    _diskPartCache->clearMessage( mailbox, uid );
}

QStringList CombinedCache::msgFlags( const QString& mailbox, uint uid ) const
{
    return _sqlCache->msgFlags( mailbox, uid );
}

void CombinedCache::setMsgFlags( const QString& mailbox, uint uid, const QStringList& flags )
{
    _sqlCache->setMsgFlags( mailbox, uid, flags );
}

AbstractCache::MessageDataBundle CombinedCache::messageMetadata( const QString& mailbox, uint uid ) const
{
    return _sqlCache->messageMetadata( mailbox, uid );
}

void CombinedCache::setMessageMetadata( const QString& mailbox, uint uid, const MessageDataBundle& metadata )
{
    _sqlCache->setMessageMetadata( mailbox, uid, metadata );
}

QByteArray CombinedCache::messagePart( const QString& mailbox, uint uid, const QString& partId ) const
{
    QByteArray res = _sqlCache->messagePart( mailbox, uid, partId );
    if ( res.isEmpty() ) {
        res = _diskPartCache->messagePart( mailbox, uid, partId );
    }
    return res;
}

void CombinedCache::setMsgPart( const QString& mailbox, uint uid, const QString& partId, const QByteArray& data )
{
    if ( data.size() < 1000 ) {
        _sqlCache->setMsgPart( mailbox, uid, partId, data );
    } else {
        _diskPartCache->setMsgPart( mailbox, uid, partId, data );
    }
}

QVector<Imap::Responses::ThreadingNode> CombinedCache::messageThreading(const QString &mailbox)
{
    return _sqlCache->messageThreading(mailbox);
}

void CombinedCache::setMessageThreading(const QString &mailbox, const QVector<Imap::Responses::ThreadingNode> &threading)
{
    _sqlCache->setMessageThreading(mailbox, threading);
}

}
}
