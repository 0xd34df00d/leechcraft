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

#ifndef IMAP_OPENCONNECTIONTASK_H
#define IMAP_OPENCONNECTIONTASK_H

#include "ImapTask.h"
#include "../Model/Model.h"

namespace Imap {
namespace Mailbox {

/** @short Create new connection and make sure it reaches authenticated state

Use this task if you want to obtain a new connection which ends up in the authenticated
state. It will establish a new connection and baby-sit it until it reaches the request
authenticated state.

Obtaining capabilities, issuing STARTTLS and logging in are all handled here.
*/
class OpenConnectionTask : public ImapTask
{
Q_OBJECT
public:
    OpenConnectionTask(Model* _model);
    virtual void perform();

    virtual bool handleStateHelper(const Imap::Responses::State* const resp);
    // FIXME: reimplement handleCapability(), add some guards against "unexpected changes" to Model's implementation

protected:
    /** @short A special, internal constructor used only by Fake_OpenConnectionTask */
    OpenConnectionTask(Model* _model, void* dummy);

private:
    void handleInitialResponse(const Imap::Responses::State* const resp);

private:
    bool waitingForGreetings;
    bool gotPreauth;
    CommandHandle startTlsCmd;
    CommandHandle capabilityCmd;
    CommandHandle loginCmd;
};

}
}

#endif // IMAP_OPENCONNECTIONTASK_H
