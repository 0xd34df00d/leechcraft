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

#include "ImapTask.h"
#include "Model.h"

namespace Imap {
namespace Mailbox {

ImapTask::ImapTask( Model* _model ) :
    QObject(_model), parser(0), model(_model), _finished(false)
{
    connect( this, SIGNAL(destroyed(QObject*)), model, SLOT(slotTaskDying(QObject*)) );
}

ImapTask::~ImapTask()
{
}

void ImapTask::addDependentTask( ImapTask *task )
{
    Q_ASSERT(task);
    dependentTasks.append( task );
}

bool ImapTask::handleState( const Imap::Responses::State* const resp )
{
    handleResponseCode(resp);
    return handleStateHelper(resp);
}

bool ImapTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleCapability( const Imap::Responses::Capability* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleNumberResponse( const Imap::Responses::NumberResponse* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleList( const Imap::Responses::List* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleFlags( const Imap::Responses::Flags* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleSearch( const Imap::Responses::Search* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleStatus( const Imap::Responses::Status* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleFetch( const Imap::Responses::Fetch* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleNamespace( const Imap::Responses::Namespace* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleSort( const Imap::Responses::Sort* const resp )
{
    Q_UNUSED(resp);
    return false;
}

bool ImapTask::handleThread( const Imap::Responses::Thread* const resp )
{
    Q_UNUSED(resp);
    return false;
}

void ImapTask::_completed()
{
    log("Completed");
    _finished = true;
    Q_FOREACH( ImapTask* task, dependentTasks ) {
        if ( ! task->isFinished() )
            task->perform();
    }
    emit completed();
}

void ImapTask::handleResponseCode( const Imap::Responses::State* const resp )
{
    using namespace Imap::Responses;
    // Check for common stuff like ALERT and CAPABILITIES update
    switch ( resp->respCode ) {
        case ALERT:
            {
                emit model->alertReceived( tr("The server sent the following ALERT:\n%1").arg( resp->message ) );
            }
            break;
        case CAPABILITIES:
            {
                const RespData<QStringList>* const caps = dynamic_cast<const RespData<QStringList>* const>(
                        resp->respCodeData.data() );
                if ( caps ) {
                    model->updateCapabilities( parser, caps->data );
                }
            }
            break;
        case BADCHARSET:
        case PARSE:
            qDebug() << "The server was having troubles with parsing message data:" << resp->message;
            break;
        default:
            // do nothing here, it must be handled later
            break;
    }
}

bool ImapTask::isReadyToRun() const
{
    return false;
}

void ImapTask::die()
{
    // FIXME: shall we kill children here as well? If we don't do that, isn't that a memleak?
}

QString ImapTask::debugIdentification() const
{
    return QString();
}

void ImapTask::log(const QString &message, const LogKind kind)
{
    Q_ASSERT(model);
    Q_ASSERT(parser);
    QString dbg = debugIdentification();
    if (!dbg.isEmpty()) {
        dbg.prepend(QLatin1Char(' '));
    }
    model->logTrace(parser->parserId(), kind, metaObject()->className() + dbg, message);
}

}

}
