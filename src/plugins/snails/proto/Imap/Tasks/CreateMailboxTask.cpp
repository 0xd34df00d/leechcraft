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


#include "CreateMailboxTask.h"
#include "GetAnyConnectionTask.h"
#include "Model.h"
#include "MailboxTree.h"

namespace Imap {
namespace Mailbox {


CreateMailboxTask::CreateMailboxTask( Model* _model, const QString& _mailbox ):
    ImapTask( _model ), mailbox(_mailbox)
{
    conn = model->_taskFactory->createGetAnyConnectionTask( _model );
    conn->addDependentTask( this );
}

void CreateMailboxTask::perform()
{
    parser = conn->parser;
    Q_ASSERT( parser );
    model->accessParser( parser ).activeTasks.append( this );

    tagCreate = parser->create( mailbox );
    emit model->activityHappening( true );
}

bool CreateMailboxTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    if ( resp->tag.isEmpty() )
        return false;

    if ( resp->tag == tagCreate ) {

        if ( resp->kind == Responses::OK ) {
            emit model->mailboxCreationSucceded( mailbox );
            tagList = parser->list( QLatin1String(""), mailbox );
            emit model->activityHappening( true );
            // Don't call _completed() yet, we're going to update mbox list before that
        } else {
            log("Failed to create mailbox");
            emit model->mailboxCreationFailed( mailbox, resp->message );
            _completed();
            // FIXME: proper error handling for the Tasks API
        }
        return true;
    } else if ( resp->tag == tagList ) {
        if ( resp->kind == Responses::OK ) {
            model->_finalizeIncrementalList( parser, mailbox );
        } else {
            // FIXME
        }
        _completed();
        return true;
    } else {
        return false;
    }
}


}
}
