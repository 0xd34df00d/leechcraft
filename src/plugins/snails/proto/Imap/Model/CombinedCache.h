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

#ifndef IMAP_MODEL_COMBINEDCACHE_H
#define IMAP_MODEL_COMBINEDCACHE_H

#include "Cache.h"

namespace Imap {

namespace Mailbox {

class SQLCache;
class DiskPartCache;


/** @short A hybrid cache, using both SQLite and on-disk format

This cache servers as a thin wrapper around the SQLCache. It uses
the SQL facilities for most of the actual caching, but changes to
a file-based cache when items are bigger than a certain threshold.

In future, this should be extended with an in-memory cache (but
only after the MemoryCache rework) which should only speed-up certain
operations. This will likely be implemented when we will switch from
storing the actual data in the various TreeItem* instances.
*/
class CombinedCache : public AbstractCache {
    Q_OBJECT
public:
    /** @short Constructor

      Create new instance, using the @arg name as the name for the database connnection.
      Store all data into the @arg cacheDir directory. Actual opening of the DB connection
      is deferred till a call to the load() method.
*/
    CombinedCache( QObject* parent, const QString& name, const QString& cacheDir );

    virtual ~CombinedCache();

    virtual QList<MailboxMetadata> childMailboxes( const QString& mailbox ) const;
    virtual bool childMailboxesFresh( const QString& mailbox ) const;
    virtual void setChildMailboxes( const QString& mailbox, const QList<MailboxMetadata>& data );
    virtual void forgetChildMailboxes( const QString& mailbox );

    virtual SyncState mailboxSyncState( const QString& mailbox ) const;
    virtual void setMailboxSyncState( const QString& mailbox, const SyncState& state );

    virtual void setUidMapping( const QString& mailbox, const QList<uint>& seqToUid );
    virtual void clearUidMapping( const QString& mailbox );
    virtual QList<uint> uidMapping( const QString& mailbox ) const;

    virtual void clearAllMessages( const QString& mailbox );
    virtual void clearMessage( const QString mailbox, uint uid );

    virtual MessageDataBundle messageMetadata( const QString& mailbox, uint uid ) const;
    virtual void setMessageMetadata( const QString& mailbox, uint uid, const MessageDataBundle& metadata );

    virtual QStringList msgFlags( const QString& mailbox, uint uid ) const;
    virtual void setMsgFlags( const QString& mailbox, uint uid, const QStringList& flags );

    virtual QByteArray messagePart( const QString& mailbox, uint uid, const QString& partId ) const;
    virtual void setMsgPart( const QString& mailbox, uint uid, const QString& partId, const QByteArray& data );

    virtual QVector<Imap::Responses::ThreadingNode> messageThreading(const QString &mailbox);
    virtual void setMessageThreading(const QString &mailbox, const QVector<Imap::Responses::ThreadingNode> &threading);

    /** @short Open a connection to the cache */
    bool open();

private:
    /** @short The SQL-based cache */
    SQLCache* _sqlCache;
    /** @short Cache for bigger message parts */
    DiskPartCache* _diskPartCache;
    /** @short Name of the DB connection */
    QString _name;
    /** @short Directory to serve as a cache root */
    QString _cacheDir;
};

}

}

#endif /* IMAP_MODEL_COMBINEDCACHE_H */
