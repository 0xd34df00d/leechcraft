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


#include "FetchMsgPartTask.h"
#include "KeepMailboxOpenTask.h"
#include "Model.h"
#include "MailboxTree.h"

namespace Imap {
namespace Mailbox {

FetchMsgPartTask::FetchMsgPartTask( Model *_model, const QModelIndex &mailbox, const QList<uint> &_uids, const QStringList &_parts ):
    ImapTask( _model ), uids(_uids), parts(_parts), mailboxIndex(mailbox)
{
    Q_ASSERT(!uids.isEmpty());
    conn = model->findTaskResponsibleFor(mailboxIndex);
    conn->addDependentTask(this);
}

void FetchMsgPartTask::perform()
{
    parser = conn->parser;
    Q_ASSERT( parser );
    model->accessParser( parser ).activeTasks.append( this );

    Sequence seq = Sequence::fromList(uids);
    tag = parser->uidFetch( seq, parts );
    emit model->activityHappening( true );
}

bool FetchMsgPartTask::handleFetch( const Imap::Responses::Fetch* const resp )
{
    if ( ! mailboxIndex.isValid() )
        return false;

    TreeItemMailbox *mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( mailboxIndex.internalPointer() ) );
    Q_ASSERT(mailbox);
    model->_genericHandleFetch( mailbox, resp );
    return true;
}

bool FetchMsgPartTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    if ( resp->tag.isEmpty() )
        return false;

    if ( resp->tag == tag ) {

        if ( resp->kind == Responses::OK ) {
            log("Fetched parts", LOG_MESSAGES);
            verifyFetchingState();
            model->changeConnectionState( parser, CONN_STATE_SELECTED );
        } else {
            // FIXME: error handling
            log("Part fetch failed", LOG_MESSAGES);
        }
        _completed();
        return true;
    } else {
        return false;
    }
}

void FetchMsgPartTask::verifyFetchingState()
{
    if ( ! mailboxIndex.isValid() ) {
        log("Mailbox has disappeared, huh?", LOG_TASKS);
        return;
    }

    TreeItemMailbox *mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( mailboxIndex.internalPointer() ) );
    Q_ASSERT(mailbox);
    QList<TreeItemMessage*> messages = model->findMessagesByUids( mailbox, uids );
    Q_FOREACH( TreeItemMessage *message, messages ) {
        Q_FOREACH( const QString &partId, parts ) {
            log("Fetched part" + partId, LOG_MESSAGES);
            model->_finalizeFetchPart( mailbox, message->row() + 1, partId );
        }
    }
}

}
}
