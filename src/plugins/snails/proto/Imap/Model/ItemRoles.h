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

#ifndef IMAP_MODEL_ITEMROLES_H
#define IMAP_MODEL_ITEMROLES_H

#include <Qt>

namespace Imap {

namespace Mailbox {

/** @short Custom item data roles for IMAP */
enum {
    /** @short A "random" offset */
    RoleBase = Qt::UserRole + 666,

    /** @short Is the item already fetched? */
    RoleIsFetched,

    /** @short Name of the mailbox */
    RoleMailboxName,
    /** @short Short name of the mailbox */
    RoleShortMailboxName,
    /** @short Separator for mailboxes at the current level */
    RoleMailboxSeparator,
    /** @short Total number of messages in a mailbox */
    RoleTotalMessageCount,
    /** @short Number of unread messages in a mailbox */
    RoleUnreadMessageCount,
    /** @short Number of recent messages in a mailbox */
    RoleRecentMessageCount,
    /** @short The mailbox in question is the INBOX */
    RoleMailboxIsINBOX,
    /** @short The mailbox can be selected */
    RoleMailboxIsSelectable,
    /** @short The mailbox has child mailboxes */
    RoleMailboxHasChildmailboxes,
    /** @short Information about whether the number of messages in the mailbox has already been loaded */
    RoleMailboxNumbersFetched,
    /** @short Is anything still loading for tihs mailbox? */
    RoleMailboxItemsAreLoading,
    /** @short Current UIDVALIDITY of a mailbox */
    RoleMailboxUidValidity,

    /** @short UID of the message */
    RoleMessageUid,
    /** @short Subject of the message */
    RoleMessageSubject,
    /** @short The From addresses */
    RoleMessageFrom,
    /** @short The To addresses */
    RoleMessageTo,
    /** @short The Cc addresses */
    RoleMessageCc,
    /** @short The Bcc: addresses */
    RoleMessageBcc,
    /** @short The Sender: header */
    RoleMessageSender,
    /** @short The Reply-To: header */
    RoleMessageReplyTo,
    /** @short The Message-Id: header */
    RoleMessageMessageId,
    /** @short The In-Reply-To: header */
    RoleMessageInReplyTo,
    /** @short The message timestamp */
    RoleMessageDate,
    /** @short Size of the message */
    RoleMessageSize,
    /** @short Status of the \Seen flag */
    RoleMessageIsMarkedRead,
    /** @short Status of the \Deleted flag */
    RoleMessageIsMarkedDeleted,
    /** @short Was the message forwarded? */
    RoleMessageIsMarkedForwarded,
    /** @short Was the message replied to? */
    RoleMessageIsMarkedReplied,
    /** @short Is the message marked as a recent one? */
    RoleMessageIsMarkedRecent,
    /** @short IMAP flags of a message */
    RoleMessageFlags,
    /** @short Is the current item a root of thread with unread messages */
    RoleThreadRootWithUnreadMessages,

    /** @short Contents of a message part */
    RolePartData,
    /** @short MIME type of a message part */
    RolePartMimeType,
    /** @short Charset of a message part */
    RolePartCharset,
    /** @short Encoding of a message part */
    RolePartEncoding,
    /** @short The body-fld-id field from BODYSTRUCTURE */
    RolePartBodyFldId,
    /** @short The Content-Disposition of a message part */
    RolePartBodyDisposition,
    /** @short The file name for a message part */
    RolePartFileName,
    /** @short The size of this part, as determined from BODYSTRUCTURE */
    RolePartOctets,
    /** @short Access to the partId() function */
    RolePartId,
    /** @short Access to the partToPath() function */
    RolePartPathToPart,

    /** @short The very last role */
    RoleInvalidLastOne
};
}

}

#endif // IMAP_MODEL_ITEMROLES_H
