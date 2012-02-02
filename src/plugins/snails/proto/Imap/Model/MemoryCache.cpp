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

#include "MemoryCache.h"
#include <QDebug>
#include <QFile>

//#define CACHE_DEBUG

namespace Imap {
namespace Mailbox {

MemoryCache::MemoryCache( QObject* parent, const QString& fileName ): AbstractCache(parent), _fileName(fileName)
{
    loadData();
}

MemoryCache::~MemoryCache()
{
    saveData();
}

QList<MailboxMetadata> MemoryCache::childMailboxes( const QString& mailbox ) const
{
    return _mailboxes[ mailbox ];
}

bool MemoryCache::childMailboxesFresh( const QString& mailbox ) const
{
    return _mailboxes.contains( mailbox );
}

void MemoryCache::setChildMailboxes( const QString& mailbox, const QList<MailboxMetadata>& data )
{
#ifdef CACHE_DEBUG
    qDebug() << "setting child mailboxes for" << mailbox << "to" << data;
#endif
    _mailboxes[ mailbox ] = data;
}

void MemoryCache::forgetChildMailboxes( const QString& mailbox )
{
    for ( QMap<QString,QList<MailboxMetadata> >::iterator it = _mailboxes.begin();
          it != _mailboxes.end(); /* do nothing */ ) {
        if ( it.key().startsWith( mailbox ) ) {
#ifdef CACHE_DEBUG
                qDebug() << "forgetting about mailbox" << it.key();
#endif
            it = _mailboxes.erase( it );
        } else {
            ++it;
        }
    }
}

SyncState MemoryCache::mailboxSyncState( const QString& mailbox ) const
{
    return _syncState[ mailbox ];
}

void MemoryCache::setMailboxSyncState( const QString& mailbox, const SyncState& state )
{
#ifdef CACHE_DEBUG
    qDebug() << "setting mailbox sync state of" << mailbox << "to" << state;
#endif
    _syncState[ mailbox ] = state;
}

void MemoryCache::setUidMapping( const QString& mailbox, const QList<uint>& seqToUid )
{
#ifdef CACHE_DEBUG
    qDebug() << "saving UID mapping for" << mailbox << "to" << seqToUid;
#endif
    _seqToUid[ mailbox ] = seqToUid;
}

void MemoryCache::clearUidMapping( const QString& mailbox )
{
#ifdef CACHE_DEBUG
    qDebug() << "clearing UID mapping for" << mailbox;
#endif
    _seqToUid.remove( mailbox );
}

void MemoryCache::clearAllMessages( const QString& mailbox )
{
#ifdef CACHE_DEBUG
    qDebug() << "pruging all info for mailbox" << mailbox;
#endif
    _flags.remove( mailbox );
    _msgMetadata.remove( mailbox );
    _parts.remove( mailbox );
}

void MemoryCache::clearMessage( const QString mailbox, uint uid )
{
#ifdef CACHE_DEBUG
    qDebug() << "pruging all info for message" << mailbox << uid;
#endif
    if ( _flags.contains( mailbox ) )
        _flags[ mailbox ].remove( uid );
    if ( _msgMetadata.contains( mailbox ) )
        _msgMetadata[ mailbox ].remove( uid );
    if ( _parts.contains( mailbox ) )
        _parts[ mailbox ].remove( uid );
}

void MemoryCache::setMsgPart( const QString& mailbox, uint uid, const QString& partId, const QByteArray& data )
{
#ifdef CACHE_DEBUG
    qDebug() << "set message part" << mailbox << uid << partId << data.size();
#endif
    _parts[ mailbox ][ uid ][ partId ] = data;
}

void MemoryCache::setMsgFlags( const QString& mailbox, uint uid, const QStringList& flags )
{
#ifdef CACHE_DEBUG
    qDebug() << "set FLAGS for" << mailbox << uid << flags;
#endif
    _flags[ mailbox ][ uid ] = flags;
}

QStringList MemoryCache::msgFlags( const QString& mailbox, uint uid ) const
{
    return _flags[ mailbox ][ uid ];
}

QList<uint> MemoryCache::uidMapping( const QString& mailbox ) const
{
    return _seqToUid[ mailbox ];
}

void MemoryCache::setMessageMetadata( const QString& mailbox, uint uid, const MessageDataBundle& metadata )
{
    LightMessageDataBundle tmp;
    tmp.envelope = metadata.envelope;
    tmp.serializedBodyStructure = metadata.serializedBodyStructure;
    tmp.size = metadata.size;
    _msgMetadata[ mailbox ][ uid ] = tmp;
}

MemoryCache::MessageDataBundle MemoryCache::messageMetadata( const QString& mailbox, uint uid ) const
{
    MessageDataBundle res;
    const QMap<uint, LightMessageDataBundle>& firstLevel = _msgMetadata[ mailbox ];
    QMap<uint, LightMessageDataBundle>::const_iterator it = firstLevel.find( uid );
    if ( it == firstLevel.end() ) {
        res.uid = 0;
        return res;
    }
    res.envelope = it->envelope;
    res.serializedBodyStructure = it->serializedBodyStructure;
    res.size = it->size;
    res.uid = uid;
    return res;
}

QByteArray MemoryCache::messagePart( const QString& mailbox, uint uid, const QString& partId ) const
{
    if ( ! _parts.contains( mailbox ) )
        return QByteArray();
    const QMap<uint, QMap<QString, QByteArray> >& mailboxParts = _parts[ mailbox ];
    if ( ! mailboxParts.contains( uid ) )
        return QByteArray();
    const QMap<QString, QByteArray>& messageParts = mailboxParts[ uid ];
    if ( ! messageParts.contains( partId ) )
        return QByteArray();
    return messageParts[ partId ];
}

QVector<Imap::Responses::ThreadingNode> MemoryCache::messageThreading(const QString &mailbox)
{
    return _threads[mailbox];
}

void MemoryCache::setMessageThreading(const QString &mailbox, const QVector<Imap::Responses::ThreadingNode> &threading)
{
    _threads[mailbox] = threading;
}

bool MemoryCache::loadData()
{
    if ( ! _fileName.isEmpty() ) {
        QFile file( _fileName );
        if ( ! file.open( QIODevice::ReadOnly ) )
            return false;
        QDataStream stream( &file );
        stream >> _mailboxes >> _syncState >> _seqToUid >> _flags >> _msgMetadata >> _parts >> _threads;
        file.close();
        return true;
    }
    return false;
}

bool MemoryCache::saveData() const
{
    if ( ! _fileName.isEmpty() ) {
        QFile file( _fileName );
        if ( ! file.open( QIODevice::WriteOnly ) )
            return false;
        QDataStream stream( &file );
        stream << _mailboxes << _syncState << _seqToUid << _flags << _msgMetadata << _parts << _threads;
        file.close();
        return true;
    }
    return false;
}

}
}

QDataStream& operator>>( QDataStream& stream, Imap::Mailbox::MemoryCache::LightMessageDataBundle& x )
{
    stream >> x.envelope >> x.serializedBodyStructure >> x.size;
    return stream;
}

QDataStream& operator<<( QDataStream& stream, const Imap::Mailbox::MemoryCache::LightMessageDataBundle& x )
{
    return stream << x.envelope << x.serializedBodyStructure << x.size;
}
