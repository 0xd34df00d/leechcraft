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

#ifndef IMAP_MODEL_DISKPARTCACHE_H
#define IMAP_MODEL_DISKPARTCACHE_H

#include <QObject>

namespace Imap {

namespace Mailbox {

/** @short Cache for storing big message parts using plain files on the disk

The API is designed to be "similar" to the AbstractCache, but because certain
operations do not really make much sense (like working with a list of mailboxes),
we do not inherit from that abstract base class.
*/
class DiskPartCache : public QObject {
    Q_OBJECT
public:
    /** @short Create the cache occupying the @arg cacheDir directory */
    DiskPartCache( QObject* parent, const QString& cacheDir );

    /** @short Delete all data of message parts which belongs to that particular mailbox */
    virtual void clearAllMessages( const QString& mailbox );
    /** @short Delete all data for a particular message in the given mailbox */
    virtual void clearMessage( const QString mailbox, uint uid );

    /** @short Return data for some message part, or a null QByteArray if not found */
    virtual QByteArray messagePart( const QString& mailbox, uint uid, const QString& partId ) const;
    /** @short Store the data for a specified message part */
    virtual void setMsgPart( const QString& mailbox, uint uid, const QString& partId, const QByteArray& data );

signals:
    /** @short An error has occured while performing cache operations */
    void error( const QString& message );

private:
    /** @short Return the directory which should be used as a storage dir for a particular mailbox */
    QString dirForMailbox( const QString& mailbox ) const;

    /** @short The root directory for all caching */
    QString _cacheDir;
};

}

}

#endif /* IMAP_MODEL_DISKPARTCACHE_H */
