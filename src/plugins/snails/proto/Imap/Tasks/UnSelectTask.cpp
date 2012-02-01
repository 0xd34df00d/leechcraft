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


#include "UnSelectTask.h"
#include "KeepMailboxOpenTask.h"
#include "Model.h"
#include "MailboxTree.h"

namespace Imap {
namespace Mailbox {

UnSelectTask::UnSelectTask( Model* _model, ImapTask* parentTask ) :
    ImapTask( _model )
{
    conn = parentTask;
    parser = conn->parser;
    Q_ASSERT( parser );
}

void UnSelectTask::perform()
{
    model->accessParser( parser ).activeTasks.prepend(this);
    if (model->accessParser(parser).maintainingTask) {
        model->accessParser(parser).maintainingTask->breakPossibleIdle();
    }
    if (model->accessParser(parser).capabilities.contains("UNSELECT")) {
        unSelectTag = parser->unSelect();
    } else {
        doFakeSelect();
    }
    emit model->activityHappening(true);
}

void UnSelectTask::doFakeSelect()
{
    if (model->accessParser(parser).maintainingTask) {
        model->accessParser(parser).maintainingTask->breakPossibleIdle();
    }
    // The server does not support UNSELECT. Let's construct an unlikely-to-exist mailbox, then.
    selectMissingTag = parser->examine(QString("trojita non existing %1").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
}

bool UnSelectTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    if (!resp->tag.isEmpty()) {
        if (resp->tag == unSelectTag) {
            if (resp->kind == Responses::OK) {
                // nothing should be needed here
            } else {
                // This is really bad.
                throw MailboxException("Attempted to unselect current mailbox, but the server denied our request. "
                                       "Can't continue, to avoid possible data corruption.", *resp);
            }
            _completed();
            return true;
        } else if (resp->tag == selectMissingTag) {
            if (resp->kind == Responses::OK) {
                QTimer::singleShot(1000, this, SLOT(doFakeSelect()));
                log(tr("The emergency EXAMINE command has unexpectedly succeeded, trying to get out of here..."), LOG_MAILBOX_SYNC);
            } else {
                // This is very good :)
                _completed();
            }
            return true;
        }
    }
    QByteArray buf;
    QTextStream s(&buf);
    s << *resp;
    log("Ignoring response " + buf, LOG_MAILBOX_SYNC);
    return true;
}

bool UnSelectTask::handleNumberResponse( const Imap::Responses::NumberResponse* const resp )
{
    Q_UNUSED(resp);
    log("UnSelectTask: ignoring numeric response", LOG_MAILBOX_SYNC);
    return true;
}

bool UnSelectTask::handleFlags( const Imap::Responses::Flags* const resp )
{
    Q_UNUSED(resp);
    log("UnSelectTask: ignoring FLAGS response", LOG_MAILBOX_SYNC);
    return true;
}

bool UnSelectTask::handleSearch( const Imap::Responses::Search* const resp )
{
    Q_UNUSED(resp);
    log("UnSelectTask: ignoring SEARCH response", LOG_MAILBOX_SYNC);
    return true;
}

bool UnSelectTask::handleFetch( const Imap::Responses::Fetch* const resp )
{
    Q_UNUSED(resp);
    log("UnSelectTask: ignoring FETCH response", LOG_MAILBOX_SYNC);
    return true;
}

}
}
