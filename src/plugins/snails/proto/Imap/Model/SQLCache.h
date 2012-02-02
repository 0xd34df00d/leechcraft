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

#ifndef IMAP_MODEL_SQLCACHE_H
#define IMAP_MODEL_SQLCACHE_H

#include "Cache.h"
#include <QSqlDatabase>
#include <QSqlQuery>

class QTimer;

/** @short Namespace for IMAP interaction */
namespace Imap {

/** @short Classes for handling of mailboxes and connections */
namespace Mailbox {

/** @short A cache implementation using an sqlite database for the underlying storage

  This class should not be used on its own, as it simply puts everything into a database.
This is clearly a suboptimal way to store large binary data, like e-mail attachments. The
purpose of this class is therefore to serve as one of a few caching backends, which are
subsequently used by an intelligent cache manager.

The database layout is aimed at a regular desktop or an embedded device. It certainly is
not meant as a proper database design -- we bundle several columns together when we know
that the API will only access them as a tuple, we use a proprietary compression on them
et cetera. In short, the layout of the database is supposed to act as a quick and dumb
cache and is certainly *not* meant to be accessed by third-party applications. Please, do
consider it an opaque format.

Some ideas for improvements:
- Don't store full string mailbox names in each table, use another table for it
- Merge uid_mapping with mailbox_sync_state, and also msg_metadata with flags
- Serious embedded users might consider putting the database into a compressed filesystem,
  or using on-the-fly compression via sqlite's VFS subsystem

 */
class SQLCache : public AbstractCache {
    Q_OBJECT
public:
    SQLCache( QObject* parent );
    virtual ~SQLCache();

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
    bool open( const QString& name, const QString& fileName  );

private:
    /** @short Broadcast an error from the SQL query */
    void emitError( const QString& message, const QSqlQuery& query ) const;
    /** @short Broadcast an error from the SQL "database" */
    void emitError( const QString& message, const QSqlDatabase& database ) const;
    /** @short Broadcast a generic error */
    void emitError( const QString& message ) const;

    /** @short Blindly create all tables */
    bool _createTables();
    /** @short Initialize the prepared queries */
    bool _prepareQueries();

    /** @short We're about to touch the DB, so it might be a good time to start a transaction */
    void touchingDB();

    /** @short Initialize the database */
    void init();

private slots:
    /** @short We haven't commited for a while */
    void timeToCommit();

private:
    QSqlDatabase db;

    mutable QSqlQuery queryChildMailboxes;
    mutable QSqlQuery queryChildMailboxesFresh;
    mutable QSqlQuery querySetChildMailboxes;
    mutable QSqlQuery queryForgetChildMailboxes1;
    mutable QSqlQuery queryMailboxSyncState;
    mutable QSqlQuery querySetMailboxSyncState;
    mutable QSqlQuery queryUidMapping;
    mutable QSqlQuery querySetUidMapping;
    mutable QSqlQuery queryClearUidMapping;
    mutable QSqlQuery queryMessageMetadata;
    mutable QSqlQuery querySetMessageMetadata;
    mutable QSqlQuery queryMessageFlags;
    mutable QSqlQuery querySetMessageFlags;
    mutable QSqlQuery queryClearAllMessages1;
    mutable QSqlQuery queryClearAllMessages2;
    mutable QSqlQuery queryClearAllMessages3;
    mutable QSqlQuery queryClearMessage1;
    mutable QSqlQuery queryClearMessage2;
    mutable QSqlQuery queryClearMessage3;
    mutable QSqlQuery queryMessagePart;
    mutable QSqlQuery querySetMessagePart;
    mutable QSqlQuery queryMessageThreading;
    mutable QSqlQuery querySetMessageThreading;

    QTimer* delayedCommit;
    QTimer* tooMuchTimeWithoutCommit;
    bool inTransaction;
};

}

}

#endif /* IMAP_MODEL_SQLCACHE_H */
