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

#include <QTimer>
#include "IdleLauncher.h"
#include "KeepMailboxOpenTask.h"
#include "Model.h"

namespace Imap {
namespace Mailbox {

IdleLauncher::IdleLauncher( KeepMailboxOpenTask* parent ):
        QObject(parent), task(parent), _idling(false), _idleCommandRunning(false)
{
    delayedEnter = new QTimer( this );
    delayedEnter->setObjectName( QString::fromAscii("%1-IdleLauncher-delayedEnter").arg( task->objectName() ) );
    delayedEnter->setSingleShot( true );
    // It's a question about what timeout to set here -- if it's too long, we enter IDLE too soon, before the
    // user has a chance to click on a message, but if we set it too long, we needlessly wait too long between
    // we receive updates, and also between terminating one IDLE and starting another.
    // 6 seconds is a compromise here.
    bool ok;
    int timeout = parent->model->property( "trojita-imap-idle-delayedEnter" ).toUInt( &ok );
    if ( ! ok )
        timeout = 6 * 1000;
    delayedEnter->setInterval( timeout );
    connect( delayedEnter, SIGNAL(timeout()), this, SLOT(slotEnterIdleNow()) );
    renewal = new QTimer( this );
    renewal->setObjectName( QString::fromAscii("%1-IdleLauncher-renewal").arg( task->objectName() ) );
    renewal->setSingleShot( true );
    timeout = parent->model->property( "trojita-imap-idle-renewal" ).toUInt( &ok );
    if ( ! ok )
        timeout = 1000 * 29 * 60; // 29 minutes -- that's the longest allowed time to IDLE
    renewal->setInterval( timeout );
    connect( renewal, SIGNAL(timeout()), this, SLOT(slotTerminateLongIdle()) );
}

void IdleLauncher::slotEnterIdleNow()
{
    delayedEnter->stop();
    renewal->stop();

    if ( _idleCommandRunning ) {
        enterIdleLater();
        return;
    }

    Q_ASSERT( task->parser );
    Q_ASSERT( ! _idling );
    Q_ASSERT( ! _idleCommandRunning );
    Q_ASSERT( task->tagIdle.isEmpty() );
    task->tagIdle = task->parser->idle();
    renewal->start();
    _idling = true;
    _idleCommandRunning = true;
}

void IdleLauncher::finishIdle()
{
    Q_ASSERT( task->parser );
    Q_ASSERT( _idling );
    Q_ASSERT( _idleCommandRunning );
    renewal->stop();
    task->parser->idleDone();
    _idling = false;
}

void IdleLauncher::slotTerminateLongIdle()
{
    if ( _idling )
        finishIdle();
}

void IdleLauncher::enterIdleLater()
{
    if ( _idling )
        return;

    delayedEnter->start();
}

void IdleLauncher::die()
{
    delayedEnter->stop();
    delayedEnter->disconnect();
    renewal->stop();
    renewal->disconnect();
}

bool IdleLauncher::idling() const
{
    return _idling;
}

bool IdleLauncher::waitingForIdleTaggedTermination() const
{
    return _idleCommandRunning;
}

void IdleLauncher::idleCommandCompleted()
{
    // FIXME: these asseerts could be triggered by a rogue server...
    if ( _idling ) {
        qDebug() << "Warning: IDLE completed before we could ask for its termination...";
        _idling = false;
        renewal->stop();
        task->parser->idleMagicallyTerminatedByServer();
    }
    Q_ASSERT( _idleCommandRunning );
    _idleCommandRunning = false;
    enterIdleLater();
}

void IdleLauncher::idleCommandFailed()
{
    // FIXME: these asseerts could be triggered by a rogue server...
    Q_ASSERT( _idling );
    Q_ASSERT( _idleCommandRunning );
    renewal->stop();
    _idleCommandRunning = false;
    task->parser->idleContinuationWontCome();
    die();
}

}
}
