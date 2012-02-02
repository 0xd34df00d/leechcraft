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


#include "NumberOfMessagesTask.h"
#include "GetAnyConnectionTask.h"
#include "Model.h"
#include "MailboxTree.h"

namespace Imap {
namespace Mailbox {


NumberOfMessagesTask::NumberOfMessagesTask( Model* _model, const QModelIndex& mailbox ):
    ImapTask( _model ), mailboxIndex(mailbox)
{
    TreeItemMailbox* mailboxPtr = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( mailbox.internalPointer() ) );
    Q_ASSERT( mailboxPtr );
    conn = model->_taskFactory->createGetAnyConnectionTask( model );
    conn->addDependentTask( this );
}

void NumberOfMessagesTask::perform()
{
    parser = conn->parser;
    Q_ASSERT( parser );
    model->accessParser( parser ).activeTasks.append( this );

    if ( ! mailboxIndex.isValid() ) {
        // FIXME: add proper fix
        log("Mailbox vanished before we could ask for number of messages inside");
        _completed();
        return;
    }
    TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( mailboxIndex.internalPointer() ) );
    Q_ASSERT( mailbox );

    tag = parser->status( mailbox->mailbox(),
                          QStringList() << QLatin1String("MESSAGES") << QLatin1String("UNSEEN") << QLatin1String("RECENT") );
    emit model->activityHappening( true );
}

bool NumberOfMessagesTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    if ( resp->tag.isEmpty() )
        return false;

    if ( resp->tag == tag ) {
        // FIXME: we should probably care about how the command ended here...
        _completed();
        return true;
    } else {
        return false;
    }
}

QString NumberOfMessagesTask::debugIdentification() const
{
    if ( ! mailboxIndex.isValid() )
        return QString::fromAscii("[invalid mailboxIndex]");

    TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( mailboxIndex.internalPointer() ) );
    Q_ASSERT(mailbox);
    return QString::fromAscii("attached to %1").arg( mailbox->mailbox() );
}

}
}
