/* Copyright (C) 2007 - 2011 Jan Kundrát <jkt@flaska.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAP_OBTAINSYNCHRONIZEDMAILBOXTASK_H
#define IMAP_OBTAINSYNCHRONIZEDMAILBOXTASK_H

#include "ImapTask.h"
#include <QModelIndex>
#include "../Model/Model.h"

namespace Imap {
namespace Mailbox {

class UnSelectTask;

/** @short Create a synchronized connection to the IMAP server

Upon creation, this class will obtain a connection to the IMAP
server (either by creating new one, or simply stealing one from
an already established one, open a mailbox, fully synchronize it
and when all of the above is done, simply declare itself completed.
*/
class ObtainSynchronizedMailboxTask : public ImapTask
{
Q_OBJECT
public:
    ObtainSynchronizedMailboxTask( Model* _model, const QModelIndex& _mailboxIndex, ImapTask* parentTask );
    virtual void perform();
    virtual bool handleStateHelper( const Imap::Responses::State* const resp );
    virtual bool handleNumberResponse( const Imap::Responses::NumberResponse* const resp );
    virtual bool handleFlags( const Imap::Responses::Flags* const resp );
    virtual bool handleSearch( const Imap::Responses::Search* const resp );
    virtual bool handleFetch( const Imap::Responses::Fetch* const resp );

    typedef enum { UID_SYNC_ALL, UID_SYNC_ONLY_NEW } UidSyncingMode;

    virtual void addDependentTask( ImapTask* task );

    virtual QString debugIdentification() const;

private:
    void _finalizeSelect();
    void _fullMboxSync( TreeItemMailbox* mailbox, TreeItemMsgList* list, const SyncState& syncState );
    void _syncNoNewNoDeletions( TreeItemMailbox* mailbox, TreeItemMsgList* list, const SyncState& syncState, const QList<uint>& seqToUid );
    void _syncOnlyDeletions( TreeItemMailbox* mailbox, TreeItemMsgList* list, const SyncState& syncState );
    void _syncOnlyAdditions( TreeItemMailbox* mailbox, TreeItemMsgList* list, const SyncState& syncState, const SyncState& oldState );
    void _syncGeneric( TreeItemMailbox* mailbox, TreeItemMsgList* list, const SyncState& syncState );

    void _finalizeUidSyncAll( TreeItemMailbox* mailbox );
    void _finalizeUidSyncOnlyNew( Model *model, TreeItemMailbox* mailbox, const uint oldExists, QList<uint> &uidMap );

    void syncUids( TreeItemMailbox* mailbox, const uint lowestUidToQuery=0 );
    void syncFlags( TreeItemMailbox* mailbox );

    void notifyInterestingMessages( TreeItemMailbox *mailbox );

    bool handleResponseCodeInsideState( const Imap::Responses::State* const resp );

    /** @short Check current mailbox for validty, and take an evasive action if it disappeared

      There's a problem when going online after an outage, where the underlying TreeItemMailbox could disappear.
      This function checks the index for validity, and queues a fake "unselect" task just to make sure that
      we get out of that mailbox as soon as possible. This task will also die() in such situation.

      See issue #88 for details.

      @returns true if the current response shall be consumed
    */
    bool dieIfInvalidMailbox();

private slots:
    /** @short We're now out of that mailbox, hurray! */
    void slotUnSelectCompleted();

private:
    ImapTask* conn;
    QPersistentModelIndex mailboxIndex;
    CommandHandle selectCmd;
    CommandHandle uidSyncingCmd;
    CommandHandle flagsCmd;
    Imap::Mailbox::MailboxSyncingProgress status;
    UidSyncingMode uidSyncingMode;
    QList<uint> uidMap;

    /** @short An UNSELECT task, if active */
    UnSelectTask *unSelectTask;

    friend class KeepMailboxOpenTask; // needs access to conn because it wants to re-use its parser, yay
};

}
}

#endif // IMAP_OBTAINSYNCHRONIZEDMAILBOXTASK_H
