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
#include "MailboxMetadata.h"

namespace Imap {
namespace Mailbox {


SyncState::SyncState():
        _exists(0), _recent(0), _unSeenCount(0), _unSeenOffset(0), _uidNext(0), _uidValidity(0),
        _hasExists(false), _hasRecent(false), _hasUnSeenCount(false), _hasUnSeenOffset(false),
        _hasUidNext(false), _hasUidValidity(false), _hasFlags(false),
        _hasPermanentFlags(false)
{
}

bool SyncState::isUsableForNumbers() const
{
    return _hasExists && _hasRecent && _hasUnSeenCount;
}

bool SyncState::isUsableForSyncing() const
{
    return _hasExists && _hasUidNext && _hasUidValidity;
}

uint SyncState::exists() const
{
    return _exists;
}

void SyncState::setExists( const uint exists )
{
    _exists = exists;
    _hasExists = true;
}

QStringList SyncState::flags() const
{
    return _flags;
}

void SyncState::setFlags( const QStringList& flags )
{
    _flags = flags;
    _hasFlags = true;
}

QStringList SyncState::permanentFlags() const
{
    return _permanentFlags;
}

void SyncState::setPermanentFlags( const QStringList& permanentFlags )
{
    _permanentFlags = permanentFlags;
    _hasPermanentFlags = true;
}

uint SyncState::recent() const
{
    return _recent;
}

void SyncState::setRecent( const uint recent )
{
    _recent = recent;
    _hasRecent = true;
}

uint SyncState::uidNext() const
{
    return _uidNext;
}

void SyncState::setUidNext( const uint uidNext )
{
    _uidNext = uidNext;
    _hasUidNext = true;
}

uint SyncState::uidValidity() const
{
    return _uidValidity;
}

void SyncState::setUidValidity( const uint uidValidity )
{
    _uidValidity = uidValidity;
    _hasUidValidity = true;
}

uint SyncState::unSeenCount() const
{
    return _unSeenCount;
}

void SyncState::setUnSeenCount( const uint unSeen )
{
    _unSeenCount = unSeen;
    _hasUnSeenCount = true;
}

uint SyncState::unSeenOffset() const
{
    return _unSeenOffset;
}

void SyncState::setUnSeenOffset( const uint unSeen )
{
    _unSeenOffset = unSeen;
    _hasUnSeenOffset = true;
}

}
}


QDebug operator<<( QDebug& dbg, const Imap::Mailbox::MailboxMetadata& metadata )
{
    return dbg << metadata.mailbox << metadata.separator << metadata.flags;
}

QDebug operator<<( QDebug& dbg, const Imap::Mailbox::SyncState& state )
{
    return dbg << "UIDVALIDITY" << state.uidValidity() << "UIDNEXT" << state.uidNext() <<
            "EXISTS" << state.exists() << "UNSEEN-count" << state.unSeenCount() <<
            "UNSEEN-offset" << state.unSeenOffset() <<
            "RECENT" << state.recent() << "PERMANENTFLAGS" << state.permanentFlags();
}

QDataStream& operator>>( QDataStream& stream, Imap::Mailbox::SyncState& ss )
{
    uint i;
    QStringList list;
    stream >> i; ss.setExists( i );
    stream >> list; ss.setFlags( list );
    stream >> list; ss.setPermanentFlags( list );
    stream >> i; ss.setRecent( i );
    stream >> i; ss.setUidNext( i );
    stream >> i; ss.setUidValidity( i );
    stream >> i; ss.setUnSeenCount( i );
    stream >> i; ss.setUnSeenOffset(i);
    return stream;
}

QDataStream& operator<<( QDataStream& stream, const Imap::Mailbox::SyncState& ss )
{
    return stream << ss.exists() << ss.flags() << ss.permanentFlags() <<
            ss.recent() << ss.uidNext() << ss.uidValidity() << ss.unSeenCount() << ss.unSeenOffset();
}

QDataStream& operator>>( QDataStream& stream, Imap::Mailbox::MailboxMetadata& mm )
{
    return stream >> mm.flags >> mm.mailbox >> mm.separator;
}

QDataStream& operator<<( QDataStream& stream, const Imap::Mailbox::MailboxMetadata& mm )
{
    return stream << mm.flags << mm.mailbox << mm.separator;
}

