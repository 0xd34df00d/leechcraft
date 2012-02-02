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

#ifndef IMAP_IMAPTASK_H
#define IMAP_IMAPTASK_H

#include <QObject>
#include "../Parser/Parser.h"
#include "../Model/Logging.h"

namespace Imap {

namespace Responses {
    class State;
    class Capability;
    class NumberResponse;
    class List;
    class Flags;
    class Search;
    class Status;
    class Fetch;
    class Namespace;
    class Sort;
    class Thread;
}

namespace Mailbox {

class Model;

/** @short Parent class for all IMAP-related jobs */
class ImapTask : public QObject
{
Q_OBJECT
public:
    ImapTask( Model* _model );
    virtual ~ImapTask();

    virtual void perform() = 0;

    /** @short Used for informing the task that it should cease from performing *any* activities immediately and that it will die soon

    This is crucial for any tasks which could perform some periodical activities involving Parser*, and should
    also be implemented for those that want to restore the rest of the world to a reasonable and consistent state
    before they get killed.
    */
    virtual void die();

    virtual void addDependentTask( ImapTask* task );

    bool handleState( const Imap::Responses::State* const resp );
    virtual bool handleStateHelper( const Imap::Responses::State* const resp );
    virtual bool handleCapability( const Imap::Responses::Capability* const resp );
    virtual bool handleNumberResponse( const Imap::Responses::NumberResponse* const resp );
    virtual bool handleList( const Imap::Responses::List* const resp );
    virtual bool handleFlags( const Imap::Responses::Flags* const resp );
    virtual bool handleSearch( const Imap::Responses::Search* const resp );
    virtual bool handleStatus( const Imap::Responses::Status* const resp );
    virtual bool handleFetch( const Imap::Responses::Fetch* const resp );
    virtual bool handleNamespace( const Imap::Responses::Namespace* const resp );
    virtual bool handleSort( const Imap::Responses::Sort* const resp );
    virtual bool handleThread( const Imap::Responses::Thread* const resp );

    /** @short Return true if this task has already finished and can be safely deleted */
    bool isFinished() const { return _finished; }

    /** @short Return true if this task doesn't depend on anything can be run immediately */
    virtual bool isReadyToRun() const;

    /** @short Obtain some additional information for the purpose of this task for debugging purposes

    The meaning of this function is to be able to tell what any given Task is supposed to do. It's useful
    especially when the Model is compiled with DEBUG_TASK_ROUTING.
    */
    virtual QString debugIdentification() const;

protected:
    void _completed();

    /** @short Get a debug logger associated with this task

    Use this function to obtain a logger which can be used for recording debug information associated with the current
    task. The events will be marked with an identification of the task which triggered them, and could be accessed from
    the GUI if logging is enabled.
    */
    void log(const QString &message, const LogKind kind=LOG_TASKS);

private:
    void handleResponseCode( const Imap::Responses::State* const resp );

signals:
    /** @short This signal is emitted if the job failed in some way */
    void failed();
    /** @short This signal is emitted upon succesfull completion of a job */
    void completed();

public:
    Imap::Parser* parser;

protected:
    Model* model;
    QList<ImapTask*> dependentTasks;
    bool _finished;
};

}
}

#endif // IMAP_IMAPTASK_H
