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

#include<sstream>
#include "KeepMailboxOpenTask.h"
#include "FetchMsgMetadataTask.h"
#include "FetchMsgPartTask.h"
#include "OpenConnectionTask.h"
#include "ObtainSynchronizedMailboxTask.h"
#include "IdleLauncher.h"
#include "MailboxTree.h"
#include "Model.h"
#include "TaskFactory.h"
#include "NoopTask.h"
#include "UnSelectTask.h"

namespace Imap {
namespace Mailbox {

/*
FIXME: we should eat "* OK [CLOSED] former mailbox closed", or somehow let it fall down to the model, which shouldn't delegate it to AuthenticatedHandler
*/

KeepMailboxOpenTask::KeepMailboxOpenTask( Model* _model, const QModelIndex& _mailboxIndex, Parser* oldParser ) :
    ImapTask( _model ), mailboxIndex(_mailboxIndex), synchronizeConn(0), shouldExit(false), isRunning(false),
    shouldRunNoop(false), shouldRunIdle(false), idleLauncher(0), unSelectTask(0)
{
    Q_ASSERT( mailboxIndex.isValid() );
    Q_ASSERT( mailboxIndex.model() == model );
    TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*> ( _mailboxIndex.internalPointer() ) );
    Q_ASSERT( mailbox );

    // We're the latest KeepMailboxOpenTask, so it makes a lot of sense to add us as the active
    // maintainingTask to the target mailbox
    mailbox->maintainingTask = this;

    if ( oldParser ) {
        // We're asked to re-use an existing connection. Let's see if there's something associated with it

        // Find if there's a KeepMailboxOpenTask already associated; if it is, we have to register with it
        if ( model->accessParser( oldParser ).maintainingTask ) {
            // The parser looks busy -- some task is associated with it and has a mailbox open, so
            // let's just wait till we get a chance to play
            // Got to copy the parser information; also make sure our assumptions about who's who really hold
            parser = oldParser;
            Q_ASSERT( model->accessParser( oldParser ).maintainingTask->parser == oldParser );
            model->accessParser( oldParser ).maintainingTask->addDependentTask( this );
            synchronizeConn = model->_taskFactory->createObtainSynchronizedMailboxTask( _model, mailboxIndex, this );
            // our synchronizeConn will be triggered by that other KeepMailboxOpenTask
        } else {
            // The parser is free, or at least there's no KeepMailboxOpenTask associated with it
            // That means that we can steal its parser...
            parser = oldParser;
            // ...and simply schedule us for immediate execution. We can do that, because there's
            // no mailbox besides us in the game, yet.
            synchronizeConn = model->_taskFactory->createObtainSynchronizedMailboxTask( _model, mailboxIndex, this );
            model->accessParser( oldParser ).maintainingTask = this;
            QTimer::singleShot( 0, this, SLOT(slotPerformConnection()) );
            // We'll also register with the model, so that all other KeepMailboxOpenTask which could
            // get constructed in future know about us and don't step on our toes
        }
    } else {
        // Create new connection
        ImapTask* conn = model->_taskFactory->createOpenConnectionTask( model );
        parser = conn->parser;
        Q_ASSERT(parser);
        model->accessParser( parser ).maintainingTask = this;
        // We don't register ourselves as a "dependant task" on conn, as we don't want it to call perform() on us;
        // instead of that, we listen for its completion and trigger the slotPerformConnection()
        connect( conn, SIGNAL(completed()), this, SLOT(slotPerformConnection()) );
        synchronizeConn = model->_taskFactory->createObtainSynchronizedMailboxTask( _model, mailboxIndex, conn );
    }
    // This will make sure that synchronizeConn will call perform() on us when it's finished
    synchronizeConn->addDependentTask( this );

    // Setup the timer for NOOPing. It won't get started at this time, though.
    noopTimer = new QTimer(this);
    connect( noopTimer, SIGNAL(timeout()), this, SLOT(slotPerformNoop()) );
    bool ok;
    int timeout = model->property( "trojita-imap-noop-period" ).toUInt( &ok );
    if ( ! ok )
        timeout = 2 * 60 * 1000; // once every two minutes
    noopTimer->setInterval( timeout );
    noopTimer->setSingleShot( true );

    fetchPartTimer = new QTimer(this);
    connect(fetchPartTimer, SIGNAL(timeout()), this, SLOT(slotFetchRequestedParts()));
    timeout = model->property( "trojita-imap-delayed-fetch-part" ).toUInt( &ok );
    if ( ! ok )
        timeout = 50;
    fetchPartTimer->setInterval( timeout );
    fetchPartTimer->setSingleShot( true );

    fetchEnvelopeTimer = new QTimer(this);
    connect(fetchEnvelopeTimer, SIGNAL(timeout()), this, SLOT(slotFetchRequestedEnvelopes()));
    fetchEnvelopeTimer->setInterval(0); // message metadata is pretty important, hence an immediate fetch
    fetchEnvelopeTimer->setSingleShot(true);

    limitBytesAtOnce = model->property( "trojita-imap-limit-fetch-bytes-per-group" ).toUInt( &ok );
    if ( ! ok )
        limitBytesAtOnce = 1024 * 1024;

    limitMessagesAtOnce = model->property( "trojita-imap-limit-fetch-messages-per-group" ).toInt( &ok );
    if ( ! ok )
        limitMessagesAtOnce = 300;

    limitParallelFetchTasks = model->property( "trojita-imap-limit-parallel-fetch-tasks" ).toInt( &ok );
    if ( ! ok )
        limitParallelFetchTasks = 10;

    limitActiveTasks = model->property( "trojita-imap-limit-active-tasks" ).toInt( &ok );
    if ( ! ok )
        limitActiveTasks = 100;

    emit model->mailboxSyncingProgress( mailboxIndex, STATE_WAIT_FOR_CONN );
}

void KeepMailboxOpenTask::slotPerformConnection()
{
    Q_ASSERT( synchronizeConn );
    Q_ASSERT( ! synchronizeConn->isFinished() );
    synchronizeConn->perform();
}

void KeepMailboxOpenTask::addDependentTask( ImapTask* task )
{
    Q_ASSERT( task );

    breakPossibleIdle();

    KeepMailboxOpenTask* keepTask = qobject_cast<KeepMailboxOpenTask*>( task );
    if ( keepTask ) {
        // Another KeepMailboxOpenTask would like to replace us, so we shall die, eventually.

        waitingTasks.append( keepTask );
        shouldExit = true;

        // Before we can die, though, we have to accomodate fetch requests for all envelopes and parts queued so far.
        slotFetchRequestedEnvelopes();
        slotFetchRequestedParts();

        if ( ! hasPendingInternalActions() && ( ! synchronizeConn || synchronizeConn->isFinished() ) ) {
            terminate();
        }
    } else {
        connect( task, SIGNAL(destroyed(QObject*)), this, SLOT(slotTaskDeleted(QObject*)) );
        ImapTask::addDependentTask( task );

        // If this is the first task to be queued, let's make sure it'll get run
        if ( delayedTasks.isEmpty() )
            QTimer::singleShot( 0, this, SLOT(slotActivateTasks()) );
        delayedTasks.append( task );
    }
}

void KeepMailboxOpenTask::slotTaskDeleted( QObject *object )
{
    // Now, object is no longer an ImapTask*, as this gets emitted from inside QObject's destructor. However,
    // we can't use the passed pointer directly, and therefore we have to perform the cast here. It is safe
    // to do that here, as we're only interested in raw pointer value.
    dependentTasks.removeOne( static_cast<ImapTask*>( object ) );

    QList<FetchMsgPartTask*>::iterator it = qFind( fetchPartTasks.begin(), fetchPartTasks.end(), static_cast<FetchMsgPartTask*>( object ) );
    if ( it != fetchPartTasks.end() ) {
        fetchPartTasks.erase( it );
        slotFetchRequestedParts();
    }
    QList<FetchMsgMetadataTask*>::iterator it2 = qFind(fetchMetadataTasks.begin(), fetchMetadataTasks.end(), static_cast<FetchMsgMetadataTask*>(object));
    if (it2 != fetchMetadataTasks.end()) {
        fetchMetadataTasks.erase(it2);
        slotFetchRequestedEnvelopes();
    }

    if ( shouldExit && ! hasPendingInternalActions() && ( ! synchronizeConn || synchronizeConn->isFinished() ) ) {
        terminate();
    } else if ( shouldRunNoop ) {
        // A command just completed, and NOOPing is active, so let's schedule it again
        noopTimer->start();
    } else if ( shouldRunIdle ) {
        // A command just completed and IDLE is supported, so let's queue it
        idleLauncher->enterIdleLater();
    }
    // It's possible that we can start more tasks at this time...
    activateTasks();
}

void KeepMailboxOpenTask::terminate()
{
    Q_ASSERT(dependentTasks.isEmpty());
    Q_ASSERT(requestedParts.isEmpty());
    Q_ASSERT(requestedEnvelopes.isEmpty());

    // Break periodic activities
    if ( idleLauncher ) {
        // got to break the IDLE cycle and especially make sure it won't restart
        idleLauncher->die();
    }
    shouldRunIdle = false;
    shouldRunNoop = false;
    isRunning = false;

    die();

    // Merge the lists of waiting tasks
    if ( ! waitingTasks.isEmpty() ) {
        KeepMailboxOpenTask* first = waitingTasks.takeFirst();
        first->waitingTasks = waitingTasks + first->waitingTasks;
        model->accessParser( parser ).maintainingTask = first;
        QTimer::singleShot( 0, first, SLOT(slotPerformConnection()) );
    }
    _finished = true;
    emit completed();
}

void KeepMailboxOpenTask::perform()
{
    Q_ASSERT( synchronizeConn );
    Q_ASSERT( synchronizeConn->isFinished() );
    parser = synchronizeConn->parser;
    synchronizeConn = 0; // will get deleted by Model
    Q_ASSERT( parser );

    model->accessParser( parser ).activeTasks.append( this );

    if ( ! waitingTasks.isEmpty() && ! hasPendingInternalActions() ) {
        // We're basically useless, but we have to die reasonably
        shouldExit = true;
        terminate();
        return;
    }

    isRunning = true;
    fetchPartTimer->start();
    fetchEnvelopeTimer->start();

    activateTasks();

    if ( model->accessParser( parser ).capabilitiesFresh && model->accessParser( parser ).capabilities.contains( "IDLE" ) )
        shouldRunIdle = true;
    else
        shouldRunNoop = true;

    if ( shouldRunNoop ) {
        noopTimer->start();
    } else if ( shouldRunIdle ) {
        idleLauncher = new IdleLauncher( this );
        if ( dependentTasks.isEmpty() ) {
            // There's no task yet, so we have to start IDLE now
            idleLauncher->enterIdleLater();
        }
    }
}

void KeepMailboxOpenTask::resynchronizeMailbox()
{
    if ( isRunning ) {
        // FIXME: would be cool to wait for completion of current tasks...
        // Instead of wild magic with re-creating synchronizeConn, it's way easier to
        // just have us replaced by another KeepMailboxOpenTask
        model->_taskFactory->createKeepMailboxOpenTask( model, mailboxIndex, parser );
    } else {
        // We aren't running yet, which means that the sync hadn't happened yet, and therefore
        // we don't have to do it "once again" -- it will happen automatically later on.
    }
}

bool KeepMailboxOpenTask::handleNumberResponse( const Imap::Responses::NumberResponse* const resp )
{
    if (dieIfInvalidMailbox())
        return true;

    // FIXME: add proper boundaries
    if ( ! isRunning )
        return false;

    TreeItemMailbox *mailbox = Model::mailboxForSomeItem( mailboxIndex );
    Q_ASSERT(mailbox);
    TreeItemMsgList *list = dynamic_cast<TreeItemMsgList*>(mailbox->_children[0]);
    Q_ASSERT(list);
    // FIXME: tests!
    if ( resp->kind == Imap::Responses::EXPUNGE ) {
        mailbox->handleExpunge( model, *resp );
        mailbox->syncState.setExists( mailbox->syncState.exists() - 1 );
        model->cache()->setMailboxSyncState( mailbox->mailbox(), mailbox->syncState );
        return true;
    } else if ( resp->kind == Imap::Responses::EXISTS ) {
        // This is a bit tricky -- unfortunately, we can't assume anything about the UID of new arrivals. On the other hand,
        // these messages can be referenced by (even unrequested) FETCH responses and deleted by EXPUNGE, so we really want
        // to add them to the tree.
        int newArrivals = resp->number - list->_children.size();
        if ( newArrivals < 0 ) {
            throw UnexpectedResponseReceived( "EXISTS response attempted to decrease number of messages", *resp );
        } else if ( newArrivals == 0 ) {
            // remains unchanged...
            return true;
        }
        mailbox->syncState.setExists( resp->number );
        model->cache()->clearUidMapping( mailbox->mailbox() );
        model->cache()->setMailboxSyncState(mailbox->mailbox(), mailbox->syncState);

        QModelIndex parent = list->toIndex(model);
        int offset = list->_children.size();
        model->beginInsertRows( parent, offset, resp->number - 1 );
        for ( int i = 0; i < newArrivals; ++i ) {
            TreeItemMessage * msg = new TreeItemMessage( list );
            msg->_offset = i + offset;
            list->_children << msg;
            // yes, we really have to add this message with UID 0 :(
        }
        model->endInsertRows();
        list->_totalMessageCount = resp->number;
        model->emitMessageCountChanged(mailbox);

        breakPossibleIdle();

        Q_ASSERT(list->_children.size());
        uint highestKnownUid = 0;
        for ( int i = list->_children.size() - 1; ! highestKnownUid && i >= 0; --i ) {
            highestKnownUid = static_cast<const TreeItemMessage*>(list->_children[i])->uid();
            //qDebug() << "UID disco: trying seq" << i << highestKnownUid;
        }
        newArrivalsFetch = parser->uidFetch( Sequence::startingAt(
                // Did the UID walk return a usable number?
                highestKnownUid ?
                    // Yes, we've got at least one message with a UID known -> ask for higher
                    highestKnownUid + 1
                :
                    // No messages, or no messages with valid UID -> use the UIDNEXT from the syncing state
                    // but prevent a possible invalid 0:*
                    qMax( mailbox->syncState.uidNext(), 1u )
                ), QStringList() << QLatin1String("FLAGS") );
        emit model->activityHappening( true );

        return true;
    } else if ( resp->kind == Imap::Responses::RECENT ) {
        mailbox->syncState.setRecent( resp->number );
        list->_recentMessageCount = resp->number;
        model->emitMessageCountChanged(mailbox);
        return true;
    } else {
        return false;
    }
}

bool KeepMailboxOpenTask::handleFetch( const Imap::Responses::Fetch* const resp )
{
    if (dieIfInvalidMailbox())
        return true;

    // FIXME: add proper boundaries
    if ( ! isRunning )
        return false;

    TreeItemMailbox *mailbox = Model::mailboxForSomeItem( mailboxIndex );
    Q_ASSERT(mailbox);
    model->_genericHandleFetch( mailbox, resp );
    return true;
}

void KeepMailboxOpenTask::slotPerformNoop()
{
    model->_taskFactory->createNoopTask(model, this);
}

bool KeepMailboxOpenTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    if (dieIfInvalidMailbox())
        return true;

    if ( handleResponseCodeInsideState(resp) )
        return true;

    // FIXME: checks for shouldExit and proper boundaries?

    if ( resp->tag.isEmpty() )
        return false;

    if ( resp->tag == tagIdle ) {

        Q_ASSERT( idleLauncher );
        if ( resp->kind == Responses::OK ) {
            // The IDLE got terminated for whatever reason, so we should schedule its restart
            idleLauncher->idleCommandCompleted();
        } else {
            // The IDLE command has failed. Let's assume it's a permanent error and don't request it in future.
            qDebug() << "The IDLE command has failed:" << resp->message;
            shouldRunIdle = false;
            idleLauncher->idleCommandFailed();
            idleLauncher->deleteLater();
            idleLauncher = 0;
        }
        tagIdle.clear();
        // IDLE is a special, because it's not really a native Task. Therefore, we have to duplicate the check for its completion
        // and possible termination request here.
        // FIXME: maybe rewrite IDLE to be a native task and get all the benefits for free? Any drawbacks?
        if ( shouldExit && ! hasPendingInternalActions() && ( ! synchronizeConn || synchronizeConn->isFinished() ) ) {
            terminate();
        }
        return true;
    } else if ( resp->tag == newArrivalsFetch ) {

        if ( resp->kind == Responses::OK ) {
            // FIXME: anything to do here?
        } else {
            // FIXME: handling of failure...
        }
        return true;
    } else {
        return false;
    }
}

void KeepMailboxOpenTask::die()
{
    if ( noopTimer )
        noopTimer->stop();
    if ( idleLauncher )
        idleLauncher->die();

    if ( mailboxIndex.isValid() ) {
        // Mark current mailbox as "orphaned by the housekeeping task"
        TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( mailboxIndex.internalPointer() ) );
        Q_ASSERT( mailbox );

        // We're already obsolete -> don't pretend to accept new tasks
        if ( mailbox->maintainingTask == this )
            mailbox->maintainingTask = 0;
    }
}

QString KeepMailboxOpenTask::debugIdentification() const
{
    if ( ! mailboxIndex.isValid() )
        return QString::fromAscii("[invalid mailboxIndex]");

    TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( mailboxIndex.internalPointer() ) );
    Q_ASSERT(mailbox);
    return QString::fromAscii("attached to %1%2%3").arg( mailbox->mailbox(),
                                                       ( synchronizeConn && ! synchronizeConn->isFinished() ) ? " [syncConn unfinished]" : "",
                                                       shouldExit ? " [shouldExit]" : ""
                                                       );
}

void KeepMailboxOpenTask::stopForLogout()
{
    die();
    breakPossibleIdle();
}

bool KeepMailboxOpenTask::handleSearch( const Imap::Responses::Search* const resp )
{
    if (dieIfInvalidMailbox())
        return true;

    TreeItemMailbox *mailbox = Model::mailboxForSomeItem( mailboxIndex );
    Q_ASSERT(mailbox);
    // Be sure there really are some new messages
    const SyncState& oldState = model->cache()->mailboxSyncState( mailbox->mailbox() );
    const int newArrivals = mailbox->syncState.exists() - oldState.exists();
    Q_ASSERT( newArrivals > 0 );

    if ( newArrivals != resp->items.size() ) {
        std::ostringstream ss;
        ss << "UID SEARCH ALL returned unexpected number of entries when syncing new arrivals into already synced mailbox: "
                << newArrivals << " expected, got " << resp->items.size() << std::endl;
        ss.flush();
        throw MailboxException( ss.str().c_str(), *resp );
    }
    uidMap = resp->items;
    return true;
}

bool KeepMailboxOpenTask::handleFlags( const Imap::Responses::Flags* const resp )
{
    if (dieIfInvalidMailbox())
        return true;

    // Well, there isn't much point in keeping track of these flags, but given that
    // IMAP servers are happy to send these responses even after the initial sync, we
    // better handle them explicitly here.
    TreeItemMailbox *mailbox = Model::mailboxForSomeItem( mailboxIndex );
    Q_ASSERT(mailbox);
    mailbox->syncState.setFlags( resp->flags );
    return true;
}

void KeepMailboxOpenTask::activateTasks()
{
    if ( ! isRunning )
        return;

    breakPossibleIdle();

    while ( ! delayedTasks.isEmpty() && model->accessParser( parser ).activeTasks.size() < limitActiveTasks ) {
        ImapTask *task = delayedTasks.takeFirst();
        if ( ! task->isFinished() )
            task->perform();
    }
}

void KeepMailboxOpenTask::requestPartDownload( const uint uid, const QString &partId, const uint estimatedSize )
{
    requestedParts[uid].insert( partId );
    requestedPartSizes[uid] += estimatedSize;
    if (!fetchPartTimer->isActive()) {
        fetchPartTimer->start();
    }
}

void KeepMailboxOpenTask::requestEnvelopeDownload(const uint uid)
{
    requestedEnvelopes.append(uid);
    if (!fetchEnvelopeTimer->isActive()) {
        fetchEnvelopeTimer->start();
    }
}

void KeepMailboxOpenTask::slotFetchRequestedParts()
{
    if ( requestedParts.isEmpty() )
        return;

    QMap<uint, QSet<QString> >::iterator it = requestedParts.begin();
    QSet<QString> parts = *it;

    // When asked to exit, do as much as possible and die
    while ( shouldExit || fetchPartTasks.size() < limitParallelFetchTasks ) {
        QList<uint> uids;
        uint totalSize = 0;
        while ( uids.size() < limitMessagesAtOnce && it != requestedParts.end() && totalSize < limitBytesAtOnce ) {
            if ( parts != *it )
                break;
            parts = *it;
            uids << it.key();
            it = requestedParts.erase( it );
            totalSize += requestedPartSizes.take( it.key() );
        }
        if ( uids.isEmpty() )
            return;

        fetchPartTasks << model->_taskFactory->createFetchMsgPartTask( model, mailboxIndex, uids, parts.toList() );
    }
}

void KeepMailboxOpenTask::slotFetchRequestedEnvelopes()
{
    if (requestedEnvelopes.isEmpty())
        return;

    QList<uint> fetchNow;
    if (shouldExit) {
        fetchNow = requestedEnvelopes;
        requestedEnvelopes.clear();
    } else {
        const int amount = qMin(requestedEnvelopes.size(), limitMessagesAtOnce); // FIXME: add an extra limit?
        fetchNow = requestedEnvelopes.mid(0, amount);
        requestedEnvelopes.erase(requestedEnvelopes.begin(), requestedEnvelopes.begin() + amount);
    }
    fetchMetadataTasks << model->_taskFactory->createFetchMsgMetadataTask(model, mailboxIndex, fetchNow);
}

void KeepMailboxOpenTask::breakPossibleIdle()
{
    if ( idleLauncher && idleLauncher->idling() ) {
        // If we're idling right now, we should immediately abort
        idleLauncher->finishIdle();
    }
}

bool KeepMailboxOpenTask::handleResponseCodeInsideState( const Imap::Responses::State* const resp )
{
    if (dieIfInvalidMailbox())
        return true;

    switch ( resp->respCode ) {
    case Responses::UIDNEXT:
    {
        TreeItemMailbox *mailbox = Model::mailboxForSomeItem( mailboxIndex );
        Q_ASSERT(mailbox);
        const Responses::RespData<uint>* const num = dynamic_cast<const Responses::RespData<uint>* const>( resp->respCodeData.data() );
        if ( num ) {
            mailbox->syncState.setUidNext( num->data );
            model->cache()->setMailboxSyncState(mailbox->mailbox(), mailbox->syncState);
            return true;
        } else {
            throw CantHappen( "State response has invalid UIDNEXT respCodeData", *resp );
        }
        break;
    }
    case Responses::PERMANENTFLAGS:
    // Another useless one, but we want to consume it now to prevent a warning about
    // an unhandled message
    {
        TreeItemMailbox *mailbox = Model::mailboxForSomeItem( mailboxIndex );
        Q_ASSERT(mailbox);
        const Responses::RespData<QStringList>* const num = dynamic_cast<const Responses::RespData<QStringList>* const>( resp->respCodeData.data() );
        if ( num ) {
            mailbox->syncState.setPermanentFlags( num->data );
            return true;
        } else {
            throw CantHappen( "State response has invalid PERMANENTFLAGS respCodeData", *resp );
        }
        break;
    }
    default:
        // Do nothing here
        break;
    }
    return false;
}

void KeepMailboxOpenTask::slotUnSelectCompleted()
{
    if (model->accessParser(parser).maintainingTask == this)
        model->accessParser(parser).maintainingTask = 0;

    isRunning = true;
    shouldExit = true;

    // Just got to wake them up, they won't succeed anyway
    activateTasks();
    slotFetchRequestedParts();

    if ( ! hasPendingInternalActions() && ( ! synchronizeConn || synchronizeConn->isFinished() ) ) {
        terminate();
    }
}

bool KeepMailboxOpenTask::dieIfInvalidMailbox()
{
    Q_ASSERT(!unSelectTask);
    if (mailboxIndex.isValid())
        return false;

    // See ObtainSynchronizedMailboxTask::dieIfInvalidMailbox() for details
    unSelectTask = model->_taskFactory->createUnSelectTask(model, this);
    connect(unSelectTask, SIGNAL(completed()), this, SLOT(slotUnSelectCompleted()));
    unSelectTask->perform();

    return true;
}

bool KeepMailboxOpenTask::hasPendingInternalActions() const
{
    bool hasToWaitForIdleTermination = idleLauncher ? idleLauncher->waitingForIdleTaggedTermination() : false;
    return ! (dependentTasks.isEmpty() && requestedParts.isEmpty() && requestedEnvelopes.isEmpty()) || hasToWaitForIdleTermination;
}

}
}
