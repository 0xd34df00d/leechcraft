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


#include "DeleteMailboxTask.h"
#include "GetAnyConnectionTask.h"
#include "Model.h"
#include "MailboxTree.h"

namespace Imap {
namespace Mailbox {


DeleteMailboxTask::DeleteMailboxTask( Model* _model, const QString& _mailbox ):
    ImapTask( _model ), mailbox(_mailbox)
{
    conn = model->_taskFactory->createGetAnyConnectionTask( _model );
    conn->addDependentTask( this );
}

void DeleteMailboxTask::perform()
{
    parser = conn->parser;
    Q_ASSERT( parser );
    model->accessParser( parser ).activeTasks.append( this );

    tag = parser->deleteMailbox( mailbox );
    emit model->activityHappening( true );
}

bool DeleteMailboxTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    if ( resp->tag.isEmpty() )
        return false;

    if ( resp->tag == tag ) {

        if ( resp->kind == Responses::OK ) {
            TreeItemMailbox* mailboxPtr = model->findMailboxByName( mailbox );
            if ( mailboxPtr ) {
                TreeItem* parentPtr = mailboxPtr->parent();
                QModelIndex parentIndex = parentPtr == model->_mailboxes ? QModelIndex() : parentPtr->toIndex(model);
                model->beginRemoveRows( parentIndex, mailboxPtr->row(), mailboxPtr->row() );
                mailboxPtr->parent()->_children.removeAt( mailboxPtr->row() );
                model->endRemoveRows();
                delete mailboxPtr;
            } else {
                QString buf;
                QDebug dbg(&buf);
                dbg << "The IMAP server just told us that it succeded to delete mailbox named" <<
                        mailbox << ", yet we don't know of any such mailbox. Message from the server:" <<
                        resp->message;
                log(buf);
            }
            emit model->mailboxDeletionSucceded( mailbox );
        } else {
            log("Couldn't delete mailbox");
            emit model->mailboxDeletionFailed( mailbox, resp->message );
            // FIXME: Tasks API error handling
        }
        _completed();
        return true;
    } else {
        return false;
    }
}


}
}
