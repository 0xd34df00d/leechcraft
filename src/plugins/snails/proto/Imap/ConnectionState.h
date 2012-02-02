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
#ifndef IMAP_CONNECTIONSTATE_H
#define IMAP_CONNECTIONSTATE_H

#include <QString>

namespace Imap {

    /** @short A human-readable state of the connection to the IMAP server */
    typedef enum {
        CONN_STATE_NONE, /**< @short Initial state */
        CONN_STATE_HOST_LOOKUP, /**< @short Resolving hostname */
        CONN_STATE_CONNECTING, /**< @short Connecting to the remote host or starting the process */
        CONN_STATE_STARTTLS, /**< @short Negotiating authentication */
        CONN_STATE_ESTABLISHED, /**< @short The connection is ready, including optional encryption */
        CONN_STATE_LOGIN, /**< @short Performing login */
        CONN_STATE_LOGIN_FAILED, /**< @short Failed to log in */
        CONN_STATE_AUTHENTICATED, /**< @short Logged in */
        CONN_STATE_SELECTING, /**< @short Selecting a mailbox -- initial state */
        CONN_STATE_SYNCING, /**< @short Selecting a mailbox -- performing synchronization */
        CONN_STATE_SELECTED, /**< @short Mailbox is selected and synchronized */
        CONN_STATE_FETCHING_PART, /** @short Downloading an actual body part */
        CONN_STATE_FETCHING_MSG_METADATA, /** @short Retrieving message metadata */
        CONN_STATE_LOGOUT, /**< @short Logging out */
    } ConnectionState;

    QString connectionStateToString( const ConnectionState state );

}

#endif // IMAP_CONNECTIONSTATE_H
