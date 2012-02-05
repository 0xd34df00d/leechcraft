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

#include "Fake_OpenConnectionTask.h"

namespace Imap {
namespace Mailbox {

Fake_OpenConnectionTask::Fake_OpenConnectionTask( Imap::Mailbox::Model* _model, Imap::Parser* _parser ):
        OpenConnectionTask( _model, 0 )
{
    // We really want to call the protected constructor, otherwise the OpenConnectionTask
    // would create a socket itself, and we don't want to end up there
    parser = _parser;
    QTimer::singleShot( 0, this, SLOT(slotPerform()) );
}

void Fake_OpenConnectionTask::perform()
{
    Q_ASSERT( parser );
    model->accessParser( parser ).activeTasks.append( this );
    _completed();
}

bool Fake_OpenConnectionTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    // This is a fake task, and therefore we aren't interested in any responses.
    // We have to override OpenConnectionTask's implementation.
    Q_UNUSED(resp);
    return false;
}


}
}
