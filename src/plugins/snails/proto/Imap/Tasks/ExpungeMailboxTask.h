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

#ifndef IMAP_EXPUNGEMAILBOX_TASK_H
#define IMAP_EXPUNGEMAILBOX_TASK_H

#include <QPersistentModelIndex>
#include "ImapTask.h"

namespace Imap {
namespace Mailbox {

/** @short Expunge all messages marked as \\Deleted in the mailbox

Please note that this task simply sends the EXPUNGE command; no careful flag
synchronization is performed.
 */
class ExpungeMailboxTask : public ImapTask
{
Q_OBJECT
public:
    ExpungeMailboxTask(Model* _model, const QModelIndex& mailbox);
    virtual void perform();

    virtual bool handleStateHelper(const Imap::Responses::State* const resp);
private:
    CommandHandle tag;
    ImapTask* conn;
    QPersistentModelIndex mailboxIndex;
};

}
}

#endif // IMAP_EXPUNGEMAILBOX_TASK_H
