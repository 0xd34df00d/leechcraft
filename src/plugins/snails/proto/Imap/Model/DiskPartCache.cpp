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

#include "DiskPartCache.h"
#include <QDebug>
#include <QDir>

namespace {
    /** @short Convert the QFile::FileError to a string representation */
    QString fileErrorToString( const QFile::FileError e )
    {
        switch ( e ) {
        case QFile::NoError:
            return QLatin1String("QFile::NoError");
        case QFile::ReadError:
            return QLatin1String("QFile::ReadError");
        case QFile::WriteError:
            return QLatin1String("QFile::WriteError");
        case QFile::FatalError:
            return QLatin1String("QFile::FatalError");
        case QFile::ResourceError:
            return QLatin1String("QFile::ResourceError");
        case QFile::OpenError:
            return QLatin1String("QFile::OpenError");
        case QFile::AbortError:
            return QLatin1String("QFile::AbortError");
        case QFile::TimeOutError:
            return QLatin1String("QFile::TimeOutError");
        case QFile::UnspecifiedError:
            return QLatin1String("QFile::UnspecifiedError");
        case QFile::RemoveError:
            return QLatin1String("QFile::RemoveError");
        case QFile::RenameError:
            return QLatin1String("QFile::RenameError");
        case QFile::PositionError:
            return QLatin1String("QFile::PositionError");
        case QFile::ResizeError:
            return QLatin1String("QFile::ResizeError");
        case QFile::PermissionsError:
            return QLatin1String("QFile::PermissionsError");
        case QFile::CopyError:
            return QLatin1String("QFile::CopyError");
        }
        return QObject::tr("Unrecognized QFile error");
    }
}

namespace Imap {
namespace Mailbox {

DiskPartCache::DiskPartCache( QObject* parent, const QString& cacheDir ): QObject(parent), _cacheDir(cacheDir)
{
    if ( ! _cacheDir.endsWith( QChar('/') ) )
        _cacheDir.append( QChar('/') );
}

void DiskPartCache::clearAllMessages( const QString& mailbox )
{
    QDir dir( dirForMailbox( mailbox ) );
    Q_FOREACH( const QString& fname, dir.entryList( QStringList() << QLatin1String("*.cache") ) ) {
        if ( ! dir.remove( fname ) ) {
            emit error( tr("Couldn't remove file %1 for mailbox %2").arg( fname, mailbox ) );
        }
    }
}

void DiskPartCache::clearMessage( const QString mailbox, uint uid )
{
    QDir dir( dirForMailbox( mailbox ) );
    Q_FOREACH( const QString& fname, dir.entryList( QStringList() << QString::fromAscii("%1_*.cache").arg( QString::number( uid ) ) ) ) {
        if ( ! dir.remove( fname ) ) {
            emit error( tr("Couldn't remove file %1 for message %2, mailbox %3").arg( fname, QString::number( uid ), mailbox ) );
        }
    }
}

QByteArray DiskPartCache::messagePart( const QString& mailbox, uint uid, const QString& partId ) const
{
    QFile buf( QString::fromAscii("%1/%2_%3.cache").arg( dirForMailbox( mailbox ), QString::number( uid ), partId ) );
    if ( ! buf.open( QIODevice::ReadOnly ) ) {
        return QByteArray();
    }
    return qUncompress( buf.readAll() );
}

void DiskPartCache::setMsgPart( const QString& mailbox, uint uid, const QString& partId, const QByteArray& data )
{
    QString myPath = dirForMailbox( mailbox );
    QDir dir(myPath);
    dir.mkpath( myPath );
    QString fileName = QString::fromAscii("%1/%2_%3.cache").arg( myPath, QString::number( uid ), partId );
    QFile buf( fileName );
    if ( ! buf.open( QIODevice::WriteOnly ) ) {
        emit error( tr("Couldn't save the part %1 of message %2 (mailbox %3) into file %4: %5 (%6)").arg(
                partId, QString::number(uid), mailbox, fileName, buf.errorString(), fileErrorToString( buf.error() ) ) );
    }
    buf.write( qCompress( data ) );
}

QString DiskPartCache::dirForMailbox( const QString& mailbox ) const
{
    return _cacheDir + mailbox.toUtf8().toBase64();
}

}
}

