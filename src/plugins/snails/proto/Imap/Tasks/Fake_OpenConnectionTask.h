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

#ifndef IMAP_FAKEOPENCONNECTION_TASK_H
#define IMAP_FAKEOPENCONNECTION_TASK_H

#include "OpenConnectionTask.h"
#include "../Parser/Parser.h"

namespace Imap {
namespace Mailbox {

/** @short A fake version of the OpenConnectionTask

This version is used for testing of various other tasks. Its purpose is to prevent
cluttering up of the socket/connection with irrelevant data when testing other tasks.
*/
class Fake_OpenConnectionTask: public OpenConnectionTask {
    Q_OBJECT
public:
    Fake_OpenConnectionTask(Imap::Mailbox::Model* _model, Imap::Parser* _parser);
    virtual void perform();
protected slots:
    void slotPerform() { perform(); }
private:
    bool handleStateHelper(const Imap::Responses::State* const resp);
};

}
}

#endif // IMAP_FAKEOPENCONNECTION_TASK_H
