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

#ifndef IMAP_UPDATEFLAGS_TASK_H
#define IMAP_UPDATEFLAGS_TASK_H

#include <QPersistentModelIndex>
#include "ImapTask.h"

namespace Imap {
namespace Mailbox {

class CopyMoveMessagesTask;

/** @short Update message flags for a particular message set

The purpose of this task is to make sure the IMAP flags for a set of messages from
a given mailbox are changed.
*/
class UpdateFlagsTask : public ImapTask
{
Q_OBJECT
public:
    /** @short Change flags for a message set

IMAP flags for the @arg _messages message set are changed -- the @arg _flagOperation
should be FLAGS, +FLAGS or -FLAGS (all of them optionally with the ".silent" modifier),
and the desired change (actual flags) is passed in the @arg _flags argument.
*/
    UpdateFlagsTask(Model* _model, const QModelIndexList& _messages, const QString& _flagOperation, const QString& _flags);

    /** @short Marking moved messages as deleted */
    UpdateFlagsTask(Model* _model, CopyMoveMessagesTask* copyTask, const QList<QPersistentModelIndex>& _messages,
                    const QString& _flagOperation, const QString& _flags);
    virtual void perform();

    virtual bool handleStateHelper(const Imap::Responses::State* const resp);
private:
    CommandHandle tag;
    ImapTask* conn;
    CopyMoveMessagesTask* copyMove;
    QList<QPersistentModelIndex> messages;
    QString flagOperation;
    QString flags;
};

}
}

#endif // IMAP_UPDATEFLAGS_TASK_H
