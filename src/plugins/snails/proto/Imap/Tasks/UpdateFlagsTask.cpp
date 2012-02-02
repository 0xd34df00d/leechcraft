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


#include "UpdateFlagsTask.h"
#include "KeepMailboxOpenTask.h"
#include "CopyMoveMessagesTask.h"
#include "Model.h"
#include "MailboxTree.h"

namespace Imap {
namespace Mailbox {

UpdateFlagsTask::UpdateFlagsTask(Model* _model, const QModelIndexList& _messages, const QString& _flagOperation, const QString& _flags):
    ImapTask(_model), copyMove(0), flagOperation(_flagOperation), flags(_flags)
{
    if (_messages.isEmpty()) {
        throw CantHappen( "UpdateFlagsTask called with empty message set");
    }
    Q_FOREACH(const QModelIndex& index, _messages) {
        messages << index;
    }
    QModelIndex mailboxIndex = model->findMailboxForItems(_messages);
    conn = model->findTaskResponsibleFor(mailboxIndex);
    conn->addDependentTask(this);
}

UpdateFlagsTask::UpdateFlagsTask(Model* _model, CopyMoveMessagesTask* copyTask, const QList<QPersistentModelIndex>& _messages,
                                 const QString& _flagOperation, const QString& _flags):
    ImapTask(_model), conn(0), copyMove(copyTask), messages(_messages), flagOperation(_flagOperation), flags(_flags)
{
    copyTask->addDependentTask(this);
}

void UpdateFlagsTask::perform()
{
    Q_ASSERT(conn || copyMove);
    if (conn)
        parser = conn->parser;
    else if (copyMove)
        parser = copyMove->parser;
    Q_ASSERT(parser);
    model->accessParser(parser).activeTasks.append(this);

    Sequence seq;
    bool first = true;

    Q_FOREACH(const QPersistentModelIndex& index, messages) {
        if (!index.isValid()) {
            // FIXME: add proper fix
            log("Some message got removed before we could update its flags", LOG_MESSAGES);
        } else {
            TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
            Q_ASSERT(item);
            TreeItemMessage* message = dynamic_cast<TreeItemMessage*>(item);
            Q_ASSERT(message);
            if (first) {
                seq = Sequence(message->uid());
                first = false;
            } else {
                seq.add(message->uid());
            }
        }
    }

    if (first) {
        // No valid messages
        log("All messages got removed before we could've updated their flags", LOG_MESSAGES);
        _completed();
        return;
    }

    tag = parser->uidStore(seq, flagOperation, flags);
    emit model->activityHappening(true);
}

bool UpdateFlagsTask::handleStateHelper(const Imap::Responses::State* const resp)
{
    if (resp->tag.isEmpty())
        return false;

    if (resp->tag == tag) {

        if (resp->kind == Responses::OK) {
            // nothing should be needed here
        } else {
            log("Failed to update FLAGS", LOG_MESSAGES);
            // FIXME: error handling
        }
        _completed();
        return true;
    } else {
        return false;
    }
}


}
}
