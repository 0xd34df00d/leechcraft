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

#include "SQLCache.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QTimer>
#include "Common/SqlTransactionAutoAborter.h"

//#define CACHE_DEBUG

namespace {
static int streamVersion = QDataStream::Qt_4_6;
}

namespace Imap {
namespace Mailbox {

SQLCache::SQLCache( QObject* parent ):
        AbstractCache(parent), delayedCommit(0), tooMuchTimeWithoutCommit(0), inTransaction(false)
{
}

void SQLCache::init()
{
#ifdef CACHE_DEBUG
    qDebug() << "SQLCache::init()";
#endif
    if ( delayedCommit )
        delayedCommit->deleteLater();
    Q_ASSERT(parent());
    bool ok;
    int num;
    delayedCommit = new QTimer( this );
    num = parent()->property( "trojita-sqlcache-commit-delay" ).toInt( &ok );
    if ( ! ok )
        num = 10000;
    delayedCommit->setInterval( num );
    delayedCommit->setObjectName( QString::fromAscii("delayedCommit-%1").arg( objectName() ) );
    connect( delayedCommit, SIGNAL(timeout()), this, SLOT(timeToCommit()) );
    if ( tooMuchTimeWithoutCommit )
        tooMuchTimeWithoutCommit->deleteLater();
    tooMuchTimeWithoutCommit = new QTimer( this );
    num = parent()->property( "trojita-sqlcache-commit-period" ).toInt( &ok );
    if ( ! ok )
        num = 60000;
    tooMuchTimeWithoutCommit->setInterval( num );
    tooMuchTimeWithoutCommit->setObjectName( QString::fromAscii("tooMuchTimeWithoutCommit-%1").arg( objectName() ) );
    connect( tooMuchTimeWithoutCommit, SIGNAL(timeout()), this, SLOT(timeToCommit()) );
}

SQLCache::~SQLCache()
{
    timeToCommit();
    db.close();
}

#define TROJITA_SQL_CACHE_CREATE_THREADING \
if ( ! q.exec( QLatin1String("CREATE TABLE msg_threading ( " \
                             "mailbox STRING NOT NULL PRIMARY KEY, " \
                             "threading BINARY" \
                             " )") ) ) { \
    emitError( tr("Can't create table msg_threading"), q ); \
    return false; \
}

#define TROJITA_SQL_CACHE_CREATE_SYNC_STATE \
if ( ! q.exec( QLatin1String("CREATE TABLE mailbox_sync_state ( " \
                             "mailbox STRING NOT NULL PRIMARY KEY, " \
                             "sync_state BINARY " \
                             " )") ) ) { \
    emitError( tr("Can't create table mailbox_sync_state"), q ); \
    return false; \
}

bool SQLCache::open( const QString& name, const QString& fileName )
{
#ifdef CACHE_DEBUG
    qDebug() << "SQLCache::open()";
#endif
    db = QSqlDatabase::addDatabase( QLatin1String("QSQLITE"), name );
    db.setDatabaseName( fileName );

    bool ok = db.open();
    if ( ! ok ) {
        emitError( tr("Can't open database"), db );
        return false;
    }

    Common::SqlTransactionAutoAborter txn( &db );

    QSqlRecord trojitaNames = db.record( QLatin1String("trojita") );
    if ( ! trojitaNames.contains( QLatin1String("version") ) ) {
        if ( ! _createTables() )
            return false;
    }

    QSqlQuery q( QString(), db );

    if ( ! q.exec( QLatin1String("SELECT version FROM trojita") ) ) {
        emitError( tr("Failed to verify version"), q );
        return false;
    }

    if ( ! q.first() ) {
        // we could probably relax this...
        emitError( tr("Can't determine version info"), q );
        return false;
    }

    uint version = q.value(0).toUInt();
    if ( version == 1 ) {
        TROJITA_SQL_CACHE_CREATE_THREADING
        version = 2;
        if ( ! q.exec(QLatin1String("UPDATE trojita SET version = 2;")) ) {
            emitError(tr("Failed to update cache DB scheme from v1 to v2"), q);
            return false;
        }
    }

    if (version == 2) {
        if (!q.exec(QLatin1String("DROP TABLE mailbox_sync_state;"))) {
            emitError(tr("Failed to drop old table mailbox_sync_state"));
            return false;
        }
        TROJITA_SQL_CACHE_CREATE_SYNC_STATE;
        version = 3;
        if ( ! q.exec(QLatin1String("UPDATE trojita SET version = 3;")) ) {
            emitError(tr("Failed to update cache DB scheme from v2 to v3"), q);
            return false;
        }
    }

    if ( version != 3 ) {
        emitError( tr("Unknown version"));
        return false;
    }

    txn.commit();

    if ( ! _prepareQueries() ) {
        return false;
    }
    init();
#ifdef CACHE_DEBUG
    qDebug() << "SQLCache::open() succeeded";
#endif
    return true;
}

bool SQLCache::_createTables()
{
    QSqlQuery q( QString(), db );

    if ( ! q.exec( QLatin1String("CREATE TABLE trojita ( version STRING NOT NULL )") ) ) {
        emitError( tr("Failed to prepare table structures"), q );
        return false;
    }
    if ( ! q.exec( QLatin1String("INSERT INTO trojita ( version ) VALUES ( 3 )") ) ) {
        emitError( tr("Can't store version info"), q );
        return false;
    }
    if ( ! q.exec( QLatin1String(
            "CREATE TABLE child_mailboxes ( "
            "mailbox STRING NOT NULL PRIMARY KEY, "
            "parent STRING NOT NULL, "
            "separator STRING, "
            "flags BINARY"
            ")"
            ) ) ) {
        emitError( tr("Can't create table child_mailboxes") );
        return false;
    }

    if ( ! q.exec( QLatin1String("CREATE TABLE uid_mapping ( "
                                 "mailbox STRING NOT NULL PRIMARY KEY, "
                                 "mapping BINARY"
                                 " )") ) ) {
        emitError( tr("Can't create table uid_mapping"), q );
        return false;
    }

    if ( ! q.exec( QLatin1String("CREATE TABLE msg_metadata ("
                                 "mailbox STRING NOT NULL, "
                                 "uid INT NOT NULL, "
                                 "data BINARY, "
                                 "PRIMARY KEY (mailbox, uid)"
                                 ")") ) ) {
        emitError( tr("Can't create table msg_metadata"), q );
    }

    if ( ! q.exec( QLatin1String("CREATE TABLE flags ("
                                 "mailbox STRING NOT NULL, "
                                 "uid INT NOT NULL, "
                                 "flags BINARY, "
                                 "PRIMARY KEY (mailbox, uid)"
                                 ")") ) ) {
        emitError( tr("Can't create table flags"), q );
    }

    if ( ! q.exec( QLatin1String("CREATE TABLE parts ("
                                 "mailbox STRING NOT NULL, "
                                 "uid INT NOT NULL, "
                                 "part_id BINARY, "
                                 "data BINARY, "
                                 "PRIMARY KEY (mailbox, uid, part_id)"
                                 ")") ) ) {
        emitError( tr("Can't create table parts"), q );
    }

    TROJITA_SQL_CACHE_CREATE_THREADING;
    TROJITA_SQL_CACHE_CREATE_SYNC_STATE;

    return true;
}

bool SQLCache::_prepareQueries()
{
    queryChildMailboxes = QSqlQuery(db);
    if ( ! queryChildMailboxes.prepare( "SELECT mailbox, separator, flags FROM child_mailboxes WHERE parent = ?") ) {
        emitError( tr("Failed to prepare queryChildMailboxes"), queryChildMailboxes );
        return false;
    }

    queryChildMailboxesFresh = QSqlQuery(db);
    if ( ! queryChildMailboxesFresh.prepare( QLatin1String("SELECT mailbox FROM child_mailboxes WHERE parent = ? LIMIT 1") ) ) {
        emitError( tr("Failed to prepare queryChildMailboxesFresh"), queryChildMailboxesFresh );
        return false;
    }

    querySetChildMailboxes = QSqlQuery(db);
    if ( ! querySetChildMailboxes.prepare( QLatin1String("INSERT OR REPLACE INTO child_mailboxes ( mailbox, parent, separator, flags ) VALUES (?, ?, ?, ?)") ) ) {
        emitError( tr("Failed to prepare querySetChildMailboxes"), querySetChildMailboxes );
        return false;
    }

    queryForgetChildMailboxes1 = QSqlQuery(db);
    if ( ! queryForgetChildMailboxes1.prepare( QLatin1String("DELETE FROM child_mailboxes WHERE parent = ?") ) ) {
        emitError( tr("Failed to prepare queryForgetChildMailboxes1"), queryForgetChildMailboxes1 );
        return false;
    }

    queryMailboxSyncState = QSqlQuery(db);
    if ( ! queryMailboxSyncState.prepare( QLatin1String("SELECT sync_state FROM mailbox_sync_state WHERE mailbox = ?") ) ) {
        emitError( tr("Failed to prepare queryMailboxSyncState"), queryMailboxSyncState );
        return false;
    }

    querySetMailboxSyncState = QSqlQuery(db);
    if ( ! querySetMailboxSyncState.prepare( QLatin1String("INSERT OR REPLACE INTO mailbox_sync_state "
                                                           "( mailbox, sync_state ) "
                                                           "VALUES ( ?, ? )") ) ) {
        emitError( tr("Failed to prepare querySetMailboxSyncState"), querySetMailboxSyncState );
        return false;
    }

    queryUidMapping = QSqlQuery(db);
    if ( ! queryUidMapping.prepare( QLatin1String("SELECT mapping FROM uid_mapping WHERE mailbox = ?") ) ) {
        emitError( tr("Failed to prepare queryUidMapping"), queryUidMapping );
        return false;
    }

    querySetUidMapping = QSqlQuery(db);
    if ( ! querySetUidMapping.prepare( QLatin1String("INSERT OR REPLACE INTO uid_mapping (mailbox, mapping) VALUES  ( ?, ? )") ) ) {
        emitError( tr("Failed to prepare querySetUidMapping"), querySetUidMapping );
        return false;
    }

    queryClearUidMapping = QSqlQuery(db);
    if ( ! queryClearUidMapping.prepare( QLatin1String("DELETE FROM uid_mapping WHERE mailbox = ?") ) ) {
        emitError( tr("Failed to prepare queryClearUidMapping"), queryClearUidMapping );
        return false;
    }

    queryMessageMetadata = QSqlQuery(db);
    if ( ! queryMessageMetadata.prepare( QLatin1String("SELECT data FROM msg_metadata WHERE mailbox = ? AND uid = ?") ) ) {
        emitError( tr("Failed to prepare queryMessageMetadata"), queryMessageMetadata );
        return false;
    }

    querySetMessageMetadata = QSqlQuery(db);
    if ( ! querySetMessageMetadata.prepare( QLatin1String("INSERT OR REPLACE INTO msg_metadata ( mailbox, uid, data ) VALUES ( ?, ?, ? )") ) ) {
        emitError( tr("Failed to prepare querySetMessageMetadata"), querySetMessageMetadata );
        return false;
    }

    queryMessageFlags = QSqlQuery(db);
    if ( ! queryMessageFlags.prepare( QLatin1String("SELECT flags FROM flags WHERE mailbox = ? AND uid = ?") ) ) {
        emitError( tr("Failed to prepare queryMessageFlags"), queryMessageFlags );
        return false;
    }

    querySetMessageFlags = QSqlQuery(db);
    if ( ! querySetMessageFlags.prepare( QLatin1String("INSERT OR REPLACE INTO flags ( mailbox, uid, flags ) VALUES ( ?, ?, ? )") ) ) {
        emitError( tr("Failed to prepare querySetMessageFlags"), querySetMessageFlags );
        return false;
    }

    queryClearAllMessages1 = QSqlQuery(db);
    if ( ! queryClearAllMessages1.prepare( QLatin1String("DELETE FROM msg_metadata WHERE mailbox = ?") ) ) {
        emitError( tr("Failed to prepare queryClearAllMessages1"), queryClearAllMessages1 );
        return false;
    }

    queryClearAllMessages2 = QSqlQuery(db);
    if ( ! queryClearAllMessages2.prepare( QLatin1String("DELETE FROM flags WHERE mailbox = ?") ) ) {
        emitError( tr("Failed to prepare queryClearAllMessages2"), queryClearAllMessages2 );
        return false;
    }

    queryClearAllMessages3 = QSqlQuery(db);
    if ( ! queryClearAllMessages3.prepare( QLatin1String("DELETE FROM parts WHERE mailbox = ?") ) ) {
        emitError( tr("Failed to prepare queryClearAllMessages3"), queryClearAllMessages2 );
        return false;
    }

    queryClearMessage1 = QSqlQuery(db);
    if ( ! queryClearMessage1.prepare( QLatin1String("DELETE FROM msg_metadata WHERE mailbox = ? AND uid = ?") ) ) {
        emitError( tr("Failed to prepare queryClearMessage1"), queryClearMessage1 );
        return false;
    }

    queryClearMessage2 = QSqlQuery(db);
    if ( ! queryClearMessage2.prepare( QLatin1String("DELETE FROM flags WHERE mailbox = ? AND uid = ?") ) ) {
        emitError( tr("Failed to prepare queryClearMessage2"), queryClearMessage2 );
        return false;
    }

    queryClearMessage3 = QSqlQuery(db);
    if ( ! queryClearMessage3.prepare( QLatin1String("DELETE FROM parts WHERE mailbox = ? AND uid = ?") ) ) {
        emitError( tr("Failed to prepare queryClearMessage3"), queryClearMessage3 );
        return false;
    }

    queryMessagePart = QSqlQuery(db);
    if ( ! queryMessagePart.prepare( QLatin1String("SELECT data FROM parts WHERE mailbox = ? AND uid = ? AND part_id = ?") ) ) {
        emitError( tr("Failed to prepare queryMessagePart"), queryMessagePart );
        return false;
    }

    querySetMessagePart = QSqlQuery(db);
    if ( ! querySetMessagePart.prepare( QLatin1String("INSERT OR REPLACE INTO parts ( mailbox, uid, part_id, data ) VALUES (?, ?, ?, ?)") ) ) {
        emitError( tr("Failed to prepare querySetMessagePart"), querySetMessagePart );
        return false;
    }

    queryMessageThreading = QSqlQuery(db);
    if ( ! queryMessageThreading.prepare( QLatin1String("SELECT threading FROM msg_threading WHERE mailbox = ?") ) ) {
        emitError( tr("Failed to prepare queryMessageThreading"), queryMessageThreading );
        return false;
    }

    querySetMessageThreading = QSqlQuery(db);
    if ( ! querySetMessageThreading.prepare( QLatin1String("INSERT OR REPLACE INTO msg_threading (mailbox, threading) VALUES  ( ?, ? )") ) ) {
        emitError( tr("Failed to prepare querySetMessageThreading"), querySetMessageThreading );
        return false;
    }

#ifdef CACHE_DEBUG
    qDebug() << "SQLCache::_prepareQueries() succeded";
#endif
    return true;
}

void SQLCache::emitError( const QString& message, const QSqlQuery& query ) const
{
    emitError( QString::fromAscii("SQLCache: Query Error: %1: %2").arg( message, query.lastError().text() ) );
}

void SQLCache::emitError( const QString& message, const QSqlDatabase& database ) const
{
    emitError( QString::fromAscii("SQLCache: DB Error: %1: %2").arg( message, database.lastError().text() ) );
}

void SQLCache::emitError( const QString& message ) const
{
    qDebug() << message;
    emit error( message );
}

QList<MailboxMetadata> SQLCache::childMailboxes( const QString& mailbox ) const
{
    QList<MailboxMetadata> res;
    queryChildMailboxes.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    if ( ! queryChildMailboxes.exec() ) {
        emitError( tr("Query queryChildMailboxes failed"), queryChildMailboxes );
        return res;
    }
    while ( queryChildMailboxes.next() ) {
        MailboxMetadata item;
        item.mailbox = queryChildMailboxes.value(0).toString();
        item.separator = queryChildMailboxes.value(1).toString();
        QDataStream stream( queryChildMailboxes.value(2).toByteArray() );
        stream.setVersion(streamVersion);
        stream >> item.flags;
        if ( stream.status() != QDataStream::Ok ) {
            emitError( tr("Corrupt data when reading child items for mailbox %1, line %2").arg( mailbox, item.mailbox ) );
            return QList<MailboxMetadata>();
        }
        res << item;
    }
    return res;
}

bool SQLCache::childMailboxesFresh( const QString& mailbox ) const
{
    queryChildMailboxesFresh.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    if ( ! queryChildMailboxesFresh.exec() ) {
        emitError( tr("Query queryChildMailboxesFresh failed"), queryChildMailboxesFresh );
        return false;
    }
    return queryChildMailboxesFresh.first();
}

void SQLCache::setChildMailboxes( const QString& mailbox, const QList<MailboxMetadata>& data )
{
#ifdef CACHE_DEBUG
    qDebug() << "Setting child mailboxes for" << mailbox;
#endif
    touchingDB();
    QString myMailbox = mailbox.isEmpty() ? QString::fromAscii("") : mailbox;
    QVariantList mailboxFields, parentFields, separatorFields, flagsFelds;
    Q_FOREACH( const MailboxMetadata& item, data ) {
        mailboxFields << item.mailbox;
        parentFields << myMailbox;
        separatorFields << item.separator;
        QByteArray buf;
        QDataStream stream( &buf, QIODevice::ReadWrite );
        stream.setVersion(streamVersion);
        stream << item.flags;
        flagsFelds << buf;
    }
    querySetChildMailboxes.bindValue( 0, mailboxFields );
    querySetChildMailboxes.bindValue( 1, parentFields );
    querySetChildMailboxes.bindValue( 2, separatorFields );
    querySetChildMailboxes.bindValue( 3, flagsFelds );
    if ( ! querySetChildMailboxes.execBatch() ) {
        emitError( tr("Query querySetChildMailboxes failed"), querySetChildMailboxes );
        return;
    }
}

void SQLCache::forgetChildMailboxes( const QString& mailbox )
{
#ifdef CACHE_DEBUG
    qDebug() << "Forgetting child mailboxes for" << mailbox;
#endif
    touchingDB();
    QString myMailbox = mailbox.isEmpty() ? QString::fromAscii("") : mailbox;
    queryForgetChildMailboxes1.bindValue( 0, myMailbox );
    if ( ! queryForgetChildMailboxes1.exec() ) {
        emitError( tr("Query queryForgetChildMailboxes1 failed"), queryForgetChildMailboxes1 );
    }
}

SyncState SQLCache::mailboxSyncState( const QString& mailbox ) const
{
    SyncState res;
    queryMailboxSyncState.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    if ( ! queryMailboxSyncState.exec() ) {
        emitError( tr("Query queryMailboxSyncState failed"), queryMailboxSyncState );
        return res;
    }
    if ( queryMailboxSyncState.first() ) {
        QDataStream stream(queryMailboxSyncState.value(0).toByteArray());
        stream.setVersion(streamVersion);
        stream >> res;
    }
    // "No data present" doesn't necessarily imply a problem -- it simply might not be there yet :)
    return res;
}

void SQLCache::setMailboxSyncState( const QString& mailbox, const SyncState& state )
{
#ifdef CACHE_DEBUG
    qDebug() << "Setting sync state for" << mailbox;
#endif
    touchingDB();
    querySetMailboxSyncState.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    QByteArray buf;
    QDataStream stream(&buf, QIODevice::ReadWrite);
    stream.setVersion(streamVersion);
    stream << state;
    querySetMailboxSyncState.bindValue(1, buf);
    if ( ! querySetMailboxSyncState.exec() ) {
        emitError( tr("Query querySetMailboxSyncState failed"), querySetMailboxSyncState );
        return;
    }
}

QList<uint> SQLCache::uidMapping( const QString& mailbox ) const
{
    QList<uint> res;
    queryUidMapping.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    if ( ! queryUidMapping.exec() ) {
        emitError( tr("Query queryUidMapping failed"), queryUidMapping );
        return res;
    }
    if ( queryUidMapping.first() ) {
        QDataStream stream( qUncompress( queryUidMapping.value(0).toByteArray() ) );
        stream.setVersion(streamVersion);
        stream >> res;
    }
    // "No data present" doesn't necessarily imply a problem -- it simply might not be there yet :)
    return res;
}

void SQLCache::setUidMapping( const QString& mailbox, const QList<uint>& seqToUid )
{
#ifdef CACHE_DEBUG
    qDebug() << "Setting UID mapping for" << mailbox;
#endif
    touchingDB();
    querySetUidMapping.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    QByteArray buf;
    QDataStream stream( &buf, QIODevice::ReadWrite );
    stream.setVersion(streamVersion);
    stream << seqToUid;
    querySetUidMapping.bindValue( 1, qCompress( buf ) );
    if ( ! querySetUidMapping.exec() ) {
        emitError( tr("Query querySetUidMapping failed"), querySetUidMapping );
    }
}

void SQLCache::clearUidMapping( const QString& mailbox )
{
#ifdef CACHE_DEBUG
    qDebug() << "Clearing UID mapping for" << mailbox;
#endif
    touchingDB();
    queryClearUidMapping.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    if ( ! queryClearUidMapping.exec() ) {
        emitError( tr("Query queryClearUidMapping failed"), queryClearUidMapping );
    }
}

void SQLCache::clearAllMessages( const QString& mailbox )
{
#ifdef CACHE_DEBUG
    qDebug() << "Clearing all messages from" << mailbox;
#endif
    touchingDB();
    queryClearAllMessages1.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryClearAllMessages2.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryClearAllMessages3.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    if ( ! queryClearAllMessages1.exec() ) {
        emitError( tr("Query queryClearAllMessages1 failed"), queryClearAllMessages1 );
    }
    if ( ! queryClearAllMessages2.exec() ) {
        emitError( tr("Query queryClearAllMessages2 failed"), queryClearAllMessages2 );
    }
    if ( ! queryClearAllMessages3.exec() ) {
        emitError( tr("Query queryClearAllMessages3 failed"), queryClearAllMessages3 );
    }
}

void SQLCache::clearMessage( const QString mailbox, uint uid )
{
#ifdef CACHE_DEBUG
    qDebug() << "Clearing message" << uid << "from" << mailbox;
#endif
    touchingDB();
    queryClearMessage1.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryClearMessage1.bindValue( 1, uid );
    queryClearMessage2.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryClearMessage2.bindValue( 1, uid );
    queryClearMessage3.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryClearMessage3.bindValue( 1, uid );
    if ( ! queryClearMessage1.exec() ) {
        emitError( tr("Query queryClearMessage1 failed"), queryClearMessage1 );
    }
    if ( ! queryClearMessage2.exec() ) {
        emitError( tr("Query queryClearMessage2 failed"), queryClearMessage2 );
    }
    if ( ! queryClearMessage3.exec() ) {
        emitError( tr("Query queryClearMessage3 failed"), queryClearMessage3 );
    }
}

QStringList SQLCache::msgFlags( const QString& mailbox, uint uid ) const
{
    QStringList res;
    queryMessageFlags.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryMessageFlags.bindValue( 1, uid );
    if ( ! queryMessageFlags.exec() ) {
        emitError( tr("Query queryMessageFlags failed"), queryMessageFlags );
        return res;
    }
    if ( queryMessageFlags.first() ) {
        QDataStream stream( queryMessageFlags.value(0).toByteArray() );
        stream.setVersion(streamVersion);
        stream >> res;
    }
    // "Not found" is not an error here
    return res;
}

void SQLCache::setMsgFlags( const QString& mailbox, uint uid, const QStringList& flags )
{
#ifdef CACHE_DEBUG
    qDebug() << "Updating flags for" << mailbox << uid;
#endif
    touchingDB();
    querySetMessageFlags.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    querySetMessageFlags.bindValue( 1, uid );
    QByteArray buf;
    QDataStream stream( &buf, QIODevice::ReadWrite );
    stream.setVersion(streamVersion);
    stream << flags;
    querySetMessageFlags.bindValue( 2, buf );
    if ( ! querySetMessageFlags.exec() ) {
        emitError( tr("Query querySetMessageFlags failed"), querySetMessageFlags );
    }
}

AbstractCache::MessageDataBundle SQLCache::messageMetadata( const QString& mailbox, uint uid ) const
{
    AbstractCache::MessageDataBundle res;
    queryMessageMetadata.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryMessageMetadata.bindValue( 1, uid );
    if ( ! queryMessageMetadata.exec() ) {
        emitError( tr("Query queryMessageMetadata failed"), queryMessageMetadata );
        return res;
    }
    if ( queryMessageMetadata.first() ) {
        res.uid = uid;
        QDataStream stream( qUncompress( queryMessageMetadata.value(0).toByteArray() ) );
        stream.setVersion(streamVersion);
        stream >> res.envelope >> res.size >> res.serializedBodyStructure;
    }
    // "Not found" is not an error here
    return res;
}

void SQLCache::setMessageMetadata( const QString& mailbox, uint uid, const MessageDataBundle& metadata )
{
#ifdef CACHE_DEBUG
    qDebug() << "Setting message metadata for" << uid << mailbox;
#endif
    touchingDB();
    // Order of values: mailbox, uid, data
    querySetMessageMetadata.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    querySetMessageMetadata.bindValue( 1, uid );
    QByteArray buf;
    QDataStream stream( &buf, QIODevice::ReadWrite );
    stream.setVersion(streamVersion);
    stream << metadata.envelope << metadata.size << metadata.serializedBodyStructure;
    querySetMessageMetadata.bindValue( 2, qCompress( buf ) );
    if ( ! querySetMessageMetadata.exec() ) {
        emitError( tr("Query querySetMessageMetadata failed"), querySetMessageMetadata );
    }
}

QByteArray SQLCache::messagePart( const QString& mailbox, uint uid, const QString& partId ) const
{
    QByteArray res;
    queryMessagePart.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    queryMessagePart.bindValue( 1, uid );
    queryMessagePart.bindValue( 2, partId );
    if ( ! queryMessagePart.exec() ) {
        emitError( tr("Query queryMessagePart failed"), queryMessagePart );
        return res;
    }
    if ( queryMessagePart.first() ) {
        res = qUncompress( queryMessagePart.value(0).toByteArray() );
        queryMessagePart.finish();
    }
    return res;
}

void SQLCache::setMsgPart( const QString& mailbox, uint uid, const QString& partId, const QByteArray& data )
{
#ifdef CACHE_DEBUG
    qDebug() << "Saving message part" << partId << uid << mailbox;
#endif
    touchingDB();
    querySetMessagePart.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    querySetMessagePart.bindValue( 1, uid );
    querySetMessagePart.bindValue( 2, partId );
    querySetMessagePart.bindValue( 3, qCompress( data ) );
    if ( ! querySetMessagePart.exec() ) {
        emitError( tr("Query querySetMessagePart failed"), querySetMessagePart );
    }
}

QVector<Imap::Responses::ThreadingNode> SQLCache::messageThreading(const QString &mailbox)
{
    QVector<Imap::Responses::ThreadingNode> res;
    queryMessageThreading.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    if ( ! queryMessageThreading.exec() ) {
        emitError( tr("Query queryMessageThreading failed"), queryMessageThreading );
        return res;
    }
    if ( queryMessageThreading.first() ) {
        QDataStream stream( qUncompress( queryMessageThreading.value(0).toByteArray() ) );
        stream.setVersion(streamVersion);
        stream >> res;
    }
    return res;
}

void SQLCache::setMessageThreading(const QString &mailbox, const QVector<Imap::Responses::ThreadingNode> &threading)
{
#ifdef CACHE_DEBUG
    qDebug() << "Setting threading for" << mailbox;
#endif
    touchingDB();
    querySetMessageThreading.bindValue( 0, mailbox.isEmpty() ? QString::fromAscii("") : mailbox );
    QByteArray buf;
    QDataStream stream( &buf, QIODevice::ReadWrite );
    stream.setVersion(streamVersion);
    stream << threading;
    querySetMessageThreading.bindValue( 1, qCompress( buf ) );
    if ( ! querySetMessageThreading.exec() ) {
        emitError( tr("Query querySetMessageThreading failed"), querySetMessageThreading );
    }

}

void SQLCache::touchingDB()
{
    delayedCommit->start();
    if ( ! inTransaction ) {
#ifdef CACHE_DEBUG
        qDebug() << "Starting transaction";
#endif
        inTransaction = true;
        db.transaction();
        tooMuchTimeWithoutCommit->start();
    }
}

void SQLCache::timeToCommit()
{
    if ( inTransaction ) {
#ifdef CACHE_DEBUG
        qDebug() << "Commit";
#endif
        inTransaction = false;
        db.commit();
    }
}


}
}
