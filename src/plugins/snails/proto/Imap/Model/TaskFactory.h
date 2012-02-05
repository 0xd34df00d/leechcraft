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

#ifndef IMAP_MODEL_TASKFACTORY_H
#define IMAP_MODEL_TASKFACTORY_H

#include <memory>
#include <QMap>
#include <QModelIndex>
#include "CopyMoveOperation.h"

namespace Imap {
class Parser;
namespace Mailbox {

class CopyMoveMessagesTask;
class CreateMailboxTask;
class DeleteMailboxTask;
class ExpungeMailboxTask;
class FetchMsgMetadataTask;
class FetchMsgPartTask;
class GetAnyConnectionTask;
class ImapTask;
class KeepMailboxOpenTask;
class ListChildMailboxesTask;
class NumberOfMessagesTask;
class ObtainSynchronizedMailboxTask;
class OpenConnectionTask;
class UpdateFlagsTask;
class ThreadTask;
class NoopTask;
class UnSelectTask;

class Model;
class TreeItemMailbox;
class TreeItemPart;

class TaskFactory
{
public:
    virtual ~TaskFactory();

    virtual CopyMoveMessagesTask* createCopyMoveMessagesTask( Model* _model, const QModelIndexList& _messages,
                                                              const QString& _targetMailbox, const CopyMoveOperation _op );
    virtual CreateMailboxTask* createCreateMailboxTask( Model* _model, const QString& _mailbox );
    virtual DeleteMailboxTask* createDeleteMailboxTask( Model* _model, const QString& _mailbox );
    virtual ExpungeMailboxTask* createExpungeMailboxTask( Model* _model, const QModelIndex& mailbox );
    virtual FetchMsgMetadataTask* createFetchMsgMetadataTask( Model *_model, const QModelIndex &_mailbox, const QList<uint> &_uid );
    virtual FetchMsgPartTask* createFetchMsgPartTask( Model* _model, const QModelIndex &mailbox, const QList<uint> &uids, const QStringList &parts );
    virtual GetAnyConnectionTask* createGetAnyConnectionTask( Model* _model );
    virtual KeepMailboxOpenTask* createKeepMailboxOpenTask( Model* _model, const QModelIndex& mailbox, Parser* oldParser );
    virtual ListChildMailboxesTask* createListChildMailboxesTask( Model* _model, const QModelIndex& mailbox );
    virtual NumberOfMessagesTask* createNumberOfMessagesTask( Model* _model, const QModelIndex& mailbox );
    virtual ObtainSynchronizedMailboxTask* createObtainSynchronizedMailboxTask( Model* _model, const QModelIndex& _mailboxIndex, ImapTask* parentTask );
    virtual OpenConnectionTask* createOpenConnectionTask( Model* _model );
    virtual UpdateFlagsTask* createUpdateFlagsTask( Model* _model, const QModelIndexList& _messages, const QString& _flagOperation, const QString& _flags );
    virtual UpdateFlagsTask* createUpdateFlagsTask( Model* _model, CopyMoveMessagesTask* copyTask, const QList<QPersistentModelIndex>& _messages, const QString& _flagOperation, const QString& _flags );
    virtual ThreadTask* createThreadTask( Model* _model, const QModelIndex& mailbox, const QString &_algorithm, const QStringList &_searchCriteria );
    virtual NoopTask* createNoopTask(Model* _model, ImapTask* parentTask);
    virtual UnSelectTask* createUnSelectTask(Model* _model, ImapTask* parentTask);
};

class TestingTaskFactory: public TaskFactory
{
public:
    TestingTaskFactory();
    virtual OpenConnectionTask* createOpenConnectionTask( Model* _model );
    virtual ListChildMailboxesTask* createListChildMailboxesTask( Model* _model, const QModelIndex& mailbox );
    bool fakeOpenConnectionTask;
    bool fakeListChildMailboxes;
    QMap<QString,QStringList> fakeListChildMailboxesMap;
private:
    Parser* newParser( Model* model );
};

typedef std::shared_ptr<TaskFactory> TaskFactoryPtr;

}
}

#endif // IMAP_MODEL_TASKFACTORY_H
