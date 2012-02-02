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

#ifndef IMAP_KEEPMAILBOXOPENTASK_H
#define IMAP_KEEPMAILBOXOPENTASK_H

#include "ImapTask.h"
#include <QModelIndex>

class QTimer;
class ImapModelIdleTest;

namespace Imap {

class Parser;

namespace Mailbox {

class ObtainSynchronizedMailboxTask;
class IdleLauncher;
class FetchMsgMetadataTask;
class FetchMsgPartTask;
class UnSelectTask;

/** @short Maintain a connection to a mailbox

This Task shall maintain a connection to a remote mailbox, updating the mailbox
state while receiving various messages.

Essentially, this Task is responsible for processing stuff like EXPUNGE replies
while the mailbox is selected. It's a bit special task, because it will not emit
completed() unless something else wants to re-use its Parser instance.
*/
class KeepMailboxOpenTask : public ImapTask
{
Q_OBJECT
public:
    /** @short Create new task for maintaining a mailbox

@arg mailboxIndex the new mailbox to open and keep open

@arg formerMailbox the mailbox which was kept open by the previous KeepMailboxOpenTask;
that mailbox now loses its KeepMailboxOpenTask and the underlying parser is reused for this task
*/
    KeepMailboxOpenTask( Model* _model, const QModelIndex& _mailboxIndex, Parser* oldParser );

    virtual void die();

    /** @short Similar to die(), but allow for correct abort of a possible IDLE command */
    void stopForLogout();

    /** @short Start child processes

This function is called when the synchronizing task finished succesfully, that is, when we are ready
to execute regular tasks which depend on us.
*/
    virtual void perform();

    /** @short Add any other task which somehow needs our current mailbox

This function also automatically registers the depending task in a special list which will make
sure that we won't emit finished() until all the dependant tasks have finished. This essnetially
prevents replacing an "alive" KeepMailboxOpenTask with a different one.
*/
    virtual void addDependentTask( ImapTask* task);

    /** @short Make sure to re-open the mailbox, even if it is already open */
    void resynchronizeMailbox();

    QString debugIdentification() const;

    void requestPartDownload( const uint uid, const QString &partId, const uint estimatedSize );
    /** @short Request a delayed loading of a message envelope */
    void requestEnvelopeDownload(const uint uid);

private slots:
    void slotTaskDeleted( QObject* object );

    /** @short Start mailbox synchronization process

This function is called when we know that the underlying Parser is no longer in active use
in any mailbox and that it is ready to be used for our purposes. It doesn't matter if that
happened because the older KeepMailboxOpenTask got finished or because new connection got
established and entered the authenticated state; the important part is that we should
initialize synchronization now.
*/
    void slotPerformConnection();

    virtual bool handleNumberResponse( const Imap::Responses::NumberResponse* const resp );
    virtual bool handleFetch( const Imap::Responses::Fetch* const resp );
    virtual bool handleStateHelper( const Imap::Responses::State* const resp );
    virtual bool handleSearch( const Imap::Responses::Search* const resp );
    virtual bool handleFlags( const Imap::Responses::Flags* const resp );
    bool handleResponseCodeInsideState( const Imap::Responses::State* const resp );

    void slotPerformNoop();
    void slotActivateTasks() { activateTasks(); }
    void slotFetchRequestedParts();
    /** @short Fetch the ENVELOPEs which were queued for later retrieval */
    void slotFetchRequestedEnvelopes();

    /** @short We're now out of that mailbox, hurray! */
    void slotUnSelectCompleted();

private:
    void terminate();

    /** @short Activate the dependent tasks while also limiting the rate */
    void activateTasks();

    /** @short If there's an IDLE running, be sure to stop it */
    void breakPossibleIdle();

    /** @short Check current mailbox for validity, and take an evasive action if it disappeared

    This is an equivalent of ObtainSynchronizedMailboxTask::dieIfInvalidMailbox. It will check whether
    the underlying index is still valid, and do best to detach from this mailbox if the index disappears.
    More details about why this is needed along the fix to ObtainSynchronizedMailboxTask can be found in
    issue #124.

    @see ObtainSynchronizedMailboxTask::dieIfInvalidMailbox
    */
    bool dieIfInvalidMailbox();

    /** @short Return true if this has a list of stuff to do */
    bool hasPendingInternalActions() const;

protected:
    QPersistentModelIndex mailboxIndex;
    /** @short Future maintaining tasks which are waiting for their opportunity to run */
    QList<KeepMailboxOpenTask*> waitingTasks;
    /** @short Regular tasks which weren't started yet

      This list is different from the dependentTasks, because for the dependentTasks, items are removed when the task
      ends, while for this waitingTasks we remove them immediately after they are started. The semantics of the
      waitingTasks is, therefore, to contain tasks which weren't started yet.
    */
    QList<ImapTask*> delayedTasks;
    /** @short An ImapTask that will be started to actually sync to a mailbox once the connection is free */
    ObtainSynchronizedMailboxTask* synchronizeConn;

    bool shouldExit;
    bool isRunning;

    QTimer* noopTimer;
    QTimer* fetchPartTimer;
    QTimer* fetchEnvelopeTimer;
    bool shouldRunNoop;
    bool shouldRunIdle;
    IdleLauncher* idleLauncher;
    QList<FetchMsgPartTask*> fetchPartTasks;
    QList<FetchMsgMetadataTask*> fetchMetadataTasks;
    CommandHandle tagIdle;
    CommandHandle newArrivalsFetch;
    friend class IdleLauncher;
    friend class ObtainSynchronizedMailboxTask; // needs access to slotUnSelectCompleted()
    friend class UnSelectTask; // needs access to breakPossibleIdle()
    friend class ::ImapModelIdleTest;

    QList<uint> uidMap;
    QMap<uint, QSet<QString> > requestedParts;
    QMap<uint, uint> requestedPartSizes;
    /** @short UIDs of messages with pending FetchMsgMetadataTask request

    QList is used in preference to the QSet in an attempt to maintain the order of requests. Simply ordering via UID is
    not enough because of output sorting, threads etc etc.
    */
    QList<uint> requestedEnvelopes;

    uint limitBytesAtOnce;
    int limitMessagesAtOnce;
    int limitParallelFetchTasks;
    int limitActiveTasks;

    /** @short An UNSELECT task, if active */
    UnSelectTask *unSelectTask;
};

}
}

#endif // IMAP_KEEPMAILBOXOPENTASK_H
