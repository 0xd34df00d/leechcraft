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

#ifndef IMAP_GETANYCONNECTIONTASK_H
#define IMAP_GETANYCONNECTIONTASK_H

#include "ImapTask.h"
#include "../Model/Model.h"

namespace Imap {
namespace Mailbox {

/** @short Come up with a connection that is (at least) in the authenticated state

In contrast to OpenConnectionTask, this task merely looks at any existing connection
and returns it, unless there are no connections, in which case it creates a new one.

In order to prevent some funny ordering issues, this task will sleep until no other
Tasks are using the connection in question, effectively serializing commands. Note
that this aspect of behavior can (and most likely will) change in future in order to
better accomodate command pipelining. Another thing to note is that this "sleeping"
only waits stuff which got queued before this task, and that GetAnyConnection typically
finishes immediately, so the serialization effectively isn't here.
*/
class GetAnyConnectionTask : public ImapTask
{
Q_OBJECT
public:
    GetAnyConnectionTask(Model* _model);
    virtual void perform();
    virtual bool isReadyToRun() const;
private:
    ImapTask* newConn;
};

}
}

#endif // IMAP_GETANYCONNECTIONTASK_H
