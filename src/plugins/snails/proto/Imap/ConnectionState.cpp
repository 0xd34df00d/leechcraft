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

#include <QObject>
#include "ConnectionState.h"
#include "Imap/Model/Model.h"

namespace Imap {

QString connectionStateToString( const ConnectionState state )
{
    switch ( state ) {
    case CONN_STATE_NONE:
        return QString();
    case CONN_STATE_HOST_LOOKUP:
        return Imap::Mailbox::Model::tr("Resolving hostname...");
    case CONN_STATE_CONNECTING:
        return Imap::Mailbox::Model::tr("Connecting to the IMAP server...");
    case CONN_STATE_STARTTLS:
        return Imap::Mailbox::Model::tr("Negotiating encryption...");
    case CONN_STATE_ESTABLISHED:
        return Imap::Mailbox::Model::tr("Connection established.");
    case CONN_STATE_LOGIN:
        return Imap::Mailbox::Model::tr("Logging in...");
    case CONN_STATE_LOGIN_FAILED:
        return Imap::Mailbox::Model::tr("Login failed.");
    case CONN_STATE_AUTHENTICATED:
        return Imap::Mailbox::Model::tr("Logged in.");
    case CONN_STATE_SELECTING:
        return Imap::Mailbox::Model::tr("Opening mailbox...");
    case CONN_STATE_SYNCING:
        return Imap::Mailbox::Model::tr("Synchronizing mailbox...");
    case CONN_STATE_SELECTED:
        return Imap::Mailbox::Model::tr("Mailbox opened.");
    case CONN_STATE_FETCHING_PART:
        return Imap::Mailbox::Model::tr("Downloading message...");
    case CONN_STATE_FETCHING_MSG_METADATA:
        return Imap::Mailbox::Model::tr("Downloading message structure...");
    case CONN_STATE_LOGOUT:
        return Imap::Mailbox::Model::tr("Logged out.");
    }
    Q_ASSERT(false);
    return QString();
}

}
