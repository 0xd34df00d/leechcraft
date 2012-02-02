/* Copyright (C) 2006 - 2011 Jan Kundrát <jkt@gentoo.org>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or the version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Model.h"
#include "MailboxTree.h"
#include "GetAnyConnectionTask.h"
#include "KeepMailboxOpenTask.h"
#include <QAbstractProxyModel>
#include <QAuthenticator>
#include <QCoreApplication>
#include <QDebug>
#include <QtAlgorithms>

//#define DEBUG_PERIODICALLY_DUMP_TASKS
//#define DEBUG_TASK_ROUTING

namespace {

using namespace Imap::Mailbox;

/** @short Return true iff the two mailboxes have the same name

It's an error to call this function on anything else but a mailbox.
*/
bool MailboxNamesEqual(const TreeItem* const a, const TreeItem* const b)
{
    const TreeItemMailbox* const mailboxA = dynamic_cast<const TreeItemMailbox* const>(a);
    const TreeItemMailbox* const mailboxB = dynamic_cast<const TreeItemMailbox* const>(b);
    Q_ASSERT(mailboxA);
    Q_ASSERT(mailboxB);

    return mailboxA->mailbox() == mailboxB->mailbox();
}

/** @short Mailbox name comparator to be used when sorting mailbox names

The special-case mailbox name, the "INBOX", is always sorted as the first one.
*/
bool MailboxNameComparator(const TreeItem* const a, const TreeItem* const b)
{
    const TreeItemMailbox* const mailboxA = dynamic_cast<const TreeItemMailbox* const>(a);
    const TreeItemMailbox* const mailboxB = dynamic_cast<const TreeItemMailbox* const>(b);

    if (mailboxA->mailbox() == QLatin1String("INBOX"))
        return true;
    if ( mailboxB->mailbox() == QLatin1String("INBOX"))
        return false;
    return mailboxA->mailbox().compare(mailboxB->mailbox(), Qt::CaseInsensitive) < 1;
}

bool uidComparator(const TreeItem* const a, const TreeItem* const b)
{
    const TreeItemMessage* const messageA = dynamic_cast<const TreeItemMessage* const>(a);
    const TreeItemMessage* const messageB = dynamic_cast<const TreeItemMessage* const>(b);
    Q_ASSERT(messageA);
    Q_ASSERT(messageB);

    return messageA->uid() < messageB->uid();
}

}

namespace Imap {
namespace Mailbox {

Model::Model( QObject* parent, AbstractCache* cache, SocketFactoryPtr socketFactory, TaskFactoryPtr taskFactory, bool offline ):
    // parent
    QAbstractItemModel( parent ),
    // our tools
    _cache(cache), _socketFactory(socketFactory), _taskFactory(taskFactory),
    _maxParsers(4), _mailboxes(0), _netPolicy( NETWORK_ONLINE ),
    _authenticator(0), lastParserId(0)
{
    _cache->setParent(this);
    _startTls = _socketFactory->startTlsRequired();

    _mailboxes = new TreeItemMailbox( 0 );

    _onlineMessageFetch << "ENVELOPE" << "BODYSTRUCTURE" << "RFC822.SIZE" << "UID" << "FLAGS";

    if ( offline ) {
        _netPolicy = NETWORK_OFFLINE;
        QTimer::singleShot( 0, this, SLOT(setNetworkOffline()) );
    } else {
        QTimer::singleShot( 0, this, SLOT( setNetworkOnline() ) );
    }

#ifdef DEBUG_PERIODICALLY_DUMP_TASKS
    QTimer* periodicTaskDumper = new QTimer(this);
    periodicTaskDumper->setInterval( 1000 );
    connect( periodicTaskDumper, SIGNAL(timeout()), this, SLOT(slotTasksChanged()) );
    periodicTaskDumper->start();
#endif
}

Model::~Model()
{
    delete _mailboxes;
    delete _authenticator;
}

void Model::responseReceived( Parser *parser )
{
    QMap<Parser*,ParserState>::iterator it = _parsers.find( parser );
    Q_ASSERT( it != _parsers.end() );

    while ( it.value().parser->hasResponse() ) {
        QSharedPointer<Imap::Responses::AbstractResponse> resp = it.value().parser->getResponse();
        Q_ASSERT( resp );
        try {
            /* At this point, we want to iterate over all active tasks and try them
            for processing the server's responses (the plug() method). However, this
            is rather complex -- this call to plug() could result in signals being
            emited, and certain slots connected to those signals might in turn want
            to queue more Tasks. Therefore, it->activeTasks could be modified, some
            items could be appended to it using the QList::append, which in turn could
            cause a realloc to happen, happily invalidating our iterators, and that
            kind of sucks.

            So, we have to iterate over a copy of the original list and instead of
            deleting Tasks, we store them into a temporary list. When we're done with
            processing, we walk the original list once again and simply remove all
            "deleted" items for real.

            This took me 3+ hours to track it down to what the hell was happening here,
            even though the underlying reason is simple -- QList::append() could invalidate
            existing iterators.
            */

            bool handled = false;
            QList<ImapTask*> taskSnapshot = it->activeTasks;
            QList<ImapTask*> deletedTasks;
            QList<ImapTask*>::const_iterator taskEnd = taskSnapshot.constEnd();

            // Try various tasks, perhaps it's their response. Also check if they're already finished and remove them.
            for ( QList<ImapTask*>::const_iterator taskIt = taskSnapshot.constBegin(); taskIt != taskEnd; ++taskIt ) {
                if ( ! handled ) {
                    handled = resp->plug(*taskIt);

#ifdef DEBUG_TASK_ROUTING
                    if ( handled )
                        qDebug() << "Handled by" << *taskIt << (*taskIt)->debugIdentification();
#endif
                }

                if ( (*taskIt)->isFinished() ) {
                    deletedTasks << *taskIt;
                    parsersMightBeIdling();
                }
            }

            removeDeletedTasks( deletedTasks, it->activeTasks );

            runReadyTasks();

            if ( ! handled ) {
#ifdef DEBUG_TASK_ROUTING
                qDebug() << "Handling by the Model itself";
#endif
                resp->plug( it.value().parser, this );
            }
        } catch (Imap::ImapException &e) {
            uint parserId = it->parser->parserId();
            killParser(it->parser, PARSER_KILL_HARD);
            _parsers.erase(it);
            broadcastParseError( parserId, QString::fromStdString( e.exceptionClass() ), e.what(), e.line(), e.offset() );
            parsersMightBeIdling();
            return;
        }
        if ( ! it.value().parser ) {
            // it got deleted
            _parsers.erase( it );
            break;
        }
    }
}

void Model::handleState(Imap::Parser *ptr, const Imap::Responses::State *const resp)
{
    // OK/NO/BAD/PREAUTH/BYE
    using namespace Imap::Responses;

    const QString &tag = resp->tag;

    if (!tag.isEmpty()) {
        if (tag == accessParser(ptr).logoutCmd) {
            // The LOGOUT is special, as it isn't associated with any task
            killParser(ptr, PARSER_KILL_EXPECTED);
        } else {
            // Unhandled command -- this is *extremely* weird
            throw CantHappen( "The following command should have been handled elsewhere", *resp );
        }
    } else {
        // untagged response
        // FIXME: we should probably just eat them and don't bother, as untagged OK/NO could be rather common...
        switch (resp->kind) {
        case BYE:
            killParser(ptr, PARSER_KILL_EXPECTED);
            parsersMightBeIdling();
            break;
        case OK:
            if (resp->respCode == NONE) {
                break;
            } else {
                logTrace(ptr->parserId(), LOG_OTHER, QString(), tr("Warning: unhandled untagged OK with a response code"));
                break;
            }
        case NO:
            logTrace(ptr->parserId(), LOG_OTHER, QString(), tr("Warning: unhandled untagged NO..."));
            break;
        default:
            throw UnexpectedResponseReceived( "Unhandled untagged response, sorry", *resp );
        }
    }
}

void Model::_finalizeList(Parser* parser, TreeItemMailbox* mailboxPtr)
{
    QList<TreeItem*> mailboxes;

    QList<Responses::List>& listResponses = accessParser(parser).listResponses;
    const QString prefix = mailboxPtr->mailbox() + mailboxPtr->separator();
    for (QList<Responses::List>::iterator it = listResponses.begin(); it != listResponses.end(); /* nothing */) {
        if (it->mailbox == mailboxPtr->mailbox() || it->mailbox == prefix) {
            // rubbish, ignore
            it = listResponses.erase(it);
        } else if (it->mailbox.startsWith(prefix)) {
            mailboxes << new TreeItemMailbox(mailboxPtr, *it);
            it = listResponses.erase(it);
        } else {
            // it clearly is someone else's LIST response
            ++it;
        }
    }
    qSort(mailboxes.begin(), mailboxes.end(), MailboxNameComparator);

    // Remove duplicates; would be great if this could be done in a STLish way,
    // but unfortunately std::unique won't help here (the "duped" part of the
    // sequence contains undefined items)
    if (mailboxes.size() > 1) {
        QList<TreeItem*>::iterator it = mailboxes.begin();
        // We've got to ignore the first one, that's the message list
        ++it;
        while (it != mailboxes.end()) {
            if (MailboxNamesEqual(it[-1], *it)) {
                delete *it;
                it = mailboxes.erase(it);
            } else {
                ++it;
            }
        }
    }

    QList<MailboxMetadata> metadataToCache;
    QList<TreeItemMailbox*> mailboxesWithoutChildren;
    for (QList<TreeItem*>::const_iterator it = mailboxes.begin(); it != mailboxes.end(); ++it) {
        TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>(*it);
        Q_ASSERT(mailbox);
        metadataToCache.append(mailbox->mailboxMetadata());
        if (mailbox->hasNoChildMaliboxesAlreadyKnown()) {
            mailboxesWithoutChildren << mailbox;
        }
    }
    cache()->setChildMailboxes(mailboxPtr->mailbox(), metadataToCache);
    for (QList<TreeItemMailbox*>::const_iterator it = mailboxesWithoutChildren.begin(); it != mailboxesWithoutChildren.end(); ++it)
        cache()->setChildMailboxes((*it)->mailbox(), QList<MailboxMetadata>());
    replaceChildMailboxes(mailboxPtr, mailboxes);
}

void Model::_finalizeIncrementalList( Parser* parser, const QString& parentMailboxName )
{
    TreeItemMailbox* parentMbox = findParentMailboxByName( parentMailboxName );
    if ( ! parentMbox ) {
        qDebug() << "Weird, no idea where to put the newly created mailbox" << parentMailboxName;
        return;
    }

    QList<TreeItem*> mailboxes;

    QList<Responses::List>& listResponses = accessParser( parser ).listResponses;
    for ( QList<Responses::List>::iterator it = listResponses.begin();
            it != listResponses.end(); /* nothing */ ) {
        if ( it->mailbox == parentMailboxName ) {
            mailboxes << new TreeItemMailbox( parentMbox, *it );
            it = listResponses.erase( it );
        } else {
            // it clearly is someone else's LIST response
            ++it;
        }
    }
    qSort( mailboxes.begin(), mailboxes.end(), MailboxNameComparator );

    if ( mailboxes.size() == 0) {
        qDebug() << "Weird, no matching LIST response for our prompt after CREATE";
        qDeleteAll( mailboxes );
        return;
    } else if ( mailboxes.size() > 1 ) {
        qDebug() << "Weird, too many LIST responses for our prompt after CREATE";
        qDeleteAll( mailboxes );
        return;
    }

    QList<TreeItem*>::iterator it = parentMbox->_children.begin();
    Q_ASSERT( it != parentMbox->_children.end() );
    ++it;
    while ( it != parentMbox->_children.end() && MailboxNameComparator( *it, mailboxes[0] ) )
        ++it;
    QModelIndex parentIdx = parentMbox == _mailboxes ? QModelIndex() : parentMbox->toIndex(this);
    if ( it == parentMbox->_children.end() )
        beginInsertRows( parentIdx, parentMbox->_children.size(), parentMbox->_children.size() );
    else
        beginInsertRows( parentIdx, (*it)->row(), (*it)->row() );
    parentMbox->_children.insert( it, mailboxes[0] );
    endInsertRows();
}

void Model::replaceChildMailboxes( TreeItemMailbox* mailboxPtr, const QList<TreeItem*> mailboxes )
{
    /* Previously, we would call layoutAboutToBeChanged() and layoutChanged() here, but it
       resulted in invalid memory access in the attached QSortFilterProxyModels like this one:

==23294== Invalid read of size 4
==23294==    at 0x5EA34B1: QSortFilterProxyModelPrivate::index_to_iterator(QModelIndex const&) const (qsortfilterproxymodel.cpp:191)
==23294==    by 0x5E9F8A3: QSortFilterProxyModel::parent(QModelIndex const&) const (qsortfilterproxymodel.cpp:1654)
==23294==    by 0x5C5D45D: QModelIndex::parent() const (qabstractitemmodel.h:389)
==23294==    by 0x5E47C48: QTreeView::drawRow(QPainter*, QStyleOptionViewItem const&, QModelIndex const&) const (qtreeview.cpp:1479)
==23294==    by 0x5E479D9: QTreeView::drawTree(QPainter*, QRegion const&) const (qtreeview.cpp:1441)
==23294==    by 0x5E4703A: QTreeView::paintEvent(QPaintEvent*) (qtreeview.cpp:1274)
==23294==    by 0x5810C30: QWidget::event(QEvent*) (qwidget.cpp:8346)
==23294==    by 0x5C91D03: QFrame::event(QEvent*) (qframe.cpp:557)
==23294==    by 0x5D4259C: QAbstractScrollArea::viewportEvent(QEvent*) (qabstractscrollarea.cpp:1043)
==23294==    by 0x5DFFD6E: QAbstractItemView::viewportEvent(QEvent*) (qabstractitemview.cpp:1619)
==23294==    by 0x5E46EE0: QTreeView::viewportEvent(QEvent*) (qtreeview.cpp:1256)
==23294==    by 0x5D43110: QAbstractScrollAreaPrivate::viewportEvent(QEvent*) (qabstractscrollarea_p.h:100)
==23294==  Address 0x908dbec is 20 bytes inside a block of size 24 free'd
==23294==    at 0x4024D74: operator delete(void*) (vg_replace_malloc.c:346)
==23294==    by 0x5EA5236: void qDeleteAll<QHash<QModelIndex, QSortFilterProxyModelPrivate::Mapping*>::const_iterator>(QHash<QModelIndex, QSortFilterProxyModelPrivate::Mapping*>::const_iterator, QHash<QModelIndex, QSortFilterProxyModelPrivate::Mapping*>::const_iterator) (qalgorithms.h:322)
==23294==    by 0x5EA3C06: void qDeleteAll<QHash<QModelIndex, QSortFilterProxyModelPrivate::Mapping*> >(QHash<QModelIndex, QSortFilterProxyModelPrivate::Mapping*> const&) (qalgorithms.h:330)
==23294==    by 0x5E9E64B: QSortFilterProxyModelPrivate::_q_sourceLayoutChanged() (qsortfilterproxymodel.cpp:1249)
==23294==    by 0x5EA29EC: QSortFilterProxyModel::qt_metacall(QMetaObject::Call, int, void**) (moc_qsortfilterproxymodel.cpp:133)
==23294==    by 0x80EB205: Imap::Mailbox::PrettyMailboxModel::qt_metacall(QMetaObject::Call, int, void**) (moc_PrettyMailboxModel.cpp:64)
==23294==    by 0x65D3EAD: QMetaObject::metacall(QObject*, QMetaObject::Call, int, void**) (qmetaobject.cpp:237)
==23294==    by 0x65E8D7C: QMetaObject::activate(QObject*, QMetaObject const*, int, void**) (qobject.cpp:3272)
==23294==    by 0x664A7E8: QAbstractItemModel::layoutChanged() (moc_qabstractitemmodel.cpp:161)
==23294==    by 0x664A354: QAbstractItemModel::qt_metacall(QMetaObject::Call, int, void**) (moc_qabstractitemmodel.cpp:118)
==23294==    by 0x5E9A3A9: QAbstractProxyModel::qt_metacall(QMetaObject::Call, int, void**) (moc_qabstractproxymodel.cpp:67)
==23294==    by 0x80EAF3D: Imap::Mailbox::MailboxModel::qt_metacall(QMetaObject::Call, int, void**) (moc_MailboxModel.cpp:81)

       I have no idea why something like that happens -- layoutChanged() should be a hint that the indexes are gone now, which means
       that the code should *not* use tham after that point. That's just weird.
    */

    QModelIndex parent = mailboxPtr == _mailboxes ? QModelIndex() : mailboxPtr->toIndex(this);

    if ( mailboxPtr->_children.size() != 1 ) {
        // There's something besides the TreeItemMsgList and we're going to
        // overwrite them, so we have to delete them right now
        int count = mailboxPtr->rowCount( this );
        beginRemoveRows( parent, 1, count - 1 );
        QList<TreeItem*> oldItems = mailboxPtr->setChildren( QList<TreeItem*>() );
        endRemoveRows();

        qDeleteAll( oldItems );
    }

    if ( ! mailboxes.isEmpty() ) {
        beginInsertRows( parent, 1, mailboxes.size() );
        QList<TreeItem*> dummy = mailboxPtr->setChildren( mailboxes );
        endInsertRows();
        Q_ASSERT( dummy.isEmpty() );
    } else {
        QList<TreeItem*> dummy = mailboxPtr->setChildren( mailboxes );
        Q_ASSERT( dummy.isEmpty() );
    }
    emit dataChanged( parent, parent );
}

void Model::emitMessageCountChanged( TreeItemMailbox* const mailbox )
{
    TreeItemMsgList* list = static_cast<TreeItemMsgList*>(mailbox->_children[0]);
    QModelIndex msgListIndex = list->toIndex(this);
    emit dataChanged(msgListIndex, msgListIndex);
    emit messageCountPossiblyChanged(mailbox->toIndex(this));
}

/** @short Retrieval of a message part has completed */
void Model::_finalizeFetchPart( TreeItemMailbox* const mailbox, const uint sequenceNo, const QString &partId )
{
    // At first, verify that the message itself is marked as loaded.
    // If it isn't, it's probably because of Model::releaseMessageData().
    TreeItem* item = mailbox->_children[0]; // TreeItemMsgList
    Q_ASSERT( static_cast<TreeItemMsgList*>( item )->fetched() );
    item = item->child( sequenceNo - 1, this ); // TreeItemMessage
    Q_ASSERT( item ); // FIXME: or rather throw an exception?
    if ( item->_fetchStatus == TreeItem::NONE ) {
        // ...and it indeed got released, so let's just return and don't try to check anything
        return;
    }

    TreeItemPart* part = mailbox->partIdToPtr( this, static_cast<TreeItemMessage*>(item), partId );
    if ( ! part ) {
        qDebug() << "Can't verify part fetching status: part is not here!";
        return;
    }
    if ( part->loading() ) {
        // basically, there's nothing to do if the FETCH targetted a message part and not the message as a whole
        qDebug() << "Imap::Model::_finalizeFetch(): didn't receive anything about message" <<
            part->message()->row() << "part" << part->partId();
        part->_fetchStatus = TreeItem::DONE;
    }
}

void Model::handleCapability( Imap::Parser* ptr, const Imap::Responses::Capability* const resp )
{
    updateCapabilities( ptr, resp->capabilities );
}

void Model::handleNumberResponse( Imap::Parser* ptr, const Imap::Responses::NumberResponse* const resp )
{
    Q_UNUSED(ptr);
    throw UnexpectedResponseReceived( "[Tasks API Port] Unhandled NumberResponse", *resp );
}

void Model::handleList( Imap::Parser* ptr, const Imap::Responses::List* const resp )
{
    accessParser( ptr ).listResponses << *resp;
}

void Model::handleFlags( Imap::Parser* ptr, const Imap::Responses::Flags* const resp )
{
    Q_UNUSED(ptr);
    throw UnexpectedResponseReceived( "[Tasks API Port] Unhandled Flags", *resp );
}

void Model::handleSearch( Imap::Parser* ptr, const Imap::Responses::Search* const resp )
{
    Q_UNUSED(ptr);
    throw UnexpectedResponseReceived( "[Tasks API Port] Unhandled Search", *resp );
}

void Model::handleStatus( Imap::Parser* ptr, const Imap::Responses::Status* const resp )
{
    Q_UNUSED( ptr );
    TreeItemMailbox* mailbox = findMailboxByName( resp->mailbox );
    if ( ! mailbox ) {
        qDebug() << "Couldn't find out which mailbox is" << resp->mailbox << "when parsing a STATUS reply";
        return;
    }
    TreeItemMsgList* list = dynamic_cast<TreeItemMsgList*>( mailbox->_children[0] );
    Q_ASSERT( list );
    if ( resp->states.contains( Imap::Responses::Status::MESSAGES ) )
        list->_totalMessageCount = resp->states[ Imap::Responses::Status::MESSAGES ];
    if ( resp->states.contains( Imap::Responses::Status::UNSEEN ) )
        list->_unreadMessageCount = resp->states[ Imap::Responses::Status::UNSEEN ];
    if ( resp->states.contains( Imap::Responses::Status::RECENT ) )
        list->_recentMessageCount = resp->states[ Imap::Responses::Status::RECENT ];
    list->_numberFetchingStatus = TreeItem::DONE;
    emitMessageCountChanged( mailbox );
}

void Model::handleFetch( Imap::Parser* ptr, const Imap::Responses::Fetch* const resp )
{
    Q_UNUSED(ptr);
    throw UnexpectedResponseReceived( "[Tasks API Port] Unhandled Fetch", *resp );
}

void Model::handleNamespace( Imap::Parser* ptr, const Imap::Responses::Namespace* const resp )
{
    return; // because it's broken and won't fly
    Q_UNUSED(ptr);
    Q_UNUSED(resp);
}

void Model::handleSort(Imap::Parser *ptr, const Imap::Responses::Sort *const resp)
{
    Q_UNUSED(ptr);
    throw UnexpectedResponseReceived( "[Tasks API Port] Unhandled Sort", *resp );
}

void Model::handleThread(Imap::Parser *ptr, const Imap::Responses::Thread *const resp)
{
    Q_UNUSED(ptr);
    throw UnexpectedResponseReceived( "[Tasks API Port] Unhandled Thread", *resp );
}

TreeItem* Model::translatePtr( const QModelIndex& index ) const
{
    return index.internalPointer() ? static_cast<TreeItem*>( index.internalPointer() ) : _mailboxes;
}

QVariant Model::data(const QModelIndex& index, int role ) const
{
    return translatePtr( index )->data( const_cast<Model*>( this ), role );
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent ) const
{
    if (parent.isValid()) {
        Q_ASSERT(parent.model() == this);
    }

    TreeItem* parentItem = translatePtr( parent );

    // Deal with the possibility of an "irregular shape" of our model here.
    // The issue is that some items have child items not only in column #0
    // and in specified number of rows, but also in row #0 and various columns.
    if ( column != 0 ) {
        TreeItem* item = parentItem->specialColumnPtr( row, column );
        if ( item )
            return QAbstractItemModel::createIndex( row, column, item );
        else
            return QModelIndex();
    }

    TreeItem* child = parentItem->child( row, const_cast<Model*>( this ) );

    return child ? QAbstractItemModel::createIndex( row, column, child ) : QModelIndex();
}

QModelIndex Model::parent(const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    Q_ASSERT(index.model() == this);

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if ( ! parentItem || parentItem == _mailboxes )
        return QModelIndex();

    return QAbstractItemModel::createIndex( parentItem->row(), 0, parentItem );
}

int Model::rowCount(const QModelIndex& index ) const
{
    TreeItem* node = static_cast<TreeItem*>( index.internalPointer() );
    if ( !node ) {
        node = _mailboxes;
    } else {
        Q_ASSERT(index.model() == this);
    }
    Q_ASSERT(node);
    return node->rowCount( const_cast<Model*>( this ) );
}

int Model::columnCount(const QModelIndex& index ) const
{
    TreeItem* node = static_cast<TreeItem*>( index.internalPointer() );
    if ( !node ) {
        node = _mailboxes;
    } else {
        Q_ASSERT(index.model() == this);
    }
    Q_ASSERT(node);
    return node->columnCount();
}

bool Model::hasChildren( const QModelIndex& parent ) const
{
    if (parent.isValid()) {
        Q_ASSERT(parent.model() == this);
    }

    TreeItem* node = translatePtr( parent );

    if ( node )
        return node->hasChildren( const_cast<Model*>( this ) );
    else
        return false;
}

void Model::_askForChildrenOfMailbox( TreeItemMailbox* item )
{
    if ( networkPolicy() != NETWORK_ONLINE && cache()->childMailboxesFresh( item->mailbox() ) ) {
        // We aren't online and the permanent cache contains relevant data
        QList<MailboxMetadata> metadata = cache()->childMailboxes( item->mailbox() );
        QList<TreeItem*> mailboxes;
        for ( QList<MailboxMetadata>::const_iterator it = metadata.begin(); it != metadata.end(); ++it ) {
            mailboxes << TreeItemMailbox::fromMetadata( item, *it );
        }
        TreeItemMailbox* mailboxPtr = dynamic_cast<TreeItemMailbox*>( item );
        Q_ASSERT( mailboxPtr );
        replaceChildMailboxes( mailboxPtr, mailboxes );
    } else if ( networkPolicy() == NETWORK_OFFLINE ) {
        // No cached data, no network -> fail
        item->_fetchStatus = TreeItem::UNAVAILABLE;
    } else {
        // We have to go to the network
        _taskFactory->createListChildMailboxesTask(this, item->toIndex(this));
    }
    QModelIndex idx = item->toIndex(this);
    emit dataChanged( idx, idx );
}

void Model::reloadMailboxList()
{
    _mailboxes->rescanForChildMailboxes( this );
}

void Model::_askForMessagesInMailbox( TreeItemMsgList* item )
{
    Q_ASSERT( item->parent() );
    TreeItemMailbox* mailboxPtr = dynamic_cast<TreeItemMailbox*>( item->parent() );
    Q_ASSERT( mailboxPtr );

    QString mailbox = mailboxPtr->mailbox();

    Q_ASSERT( item->_children.size() == 0 );

    QList<uint> uidMapping = cache()->uidMapping( mailbox );
    if ( networkPolicy() == NETWORK_OFFLINE && uidMapping.size() != item->_totalMessageCount ) {
        qDebug() << "UID cache stale for mailbox" << mailbox <<
                "(" << uidMapping.size() << "in UID cache vs." <<
                item->_totalMessageCount << "as totalMessageCount)";
        item->_fetchStatus = TreeItem::UNAVAILABLE;
    } else if ( uidMapping.size() ) {
        QModelIndex listIndex = item->toIndex(this);
        beginInsertRows( listIndex, 0, uidMapping.size() - 1 );
        for ( uint seq = 0; seq < static_cast<uint>( uidMapping.size() ); ++seq ) {
            TreeItemMessage* message = new TreeItemMessage( item );
            message->_offset = seq;
            message->_uid = uidMapping[ seq ];
            item->_children << message;
        }
        endInsertRows();
        item->_fetchStatus = TreeItem::DONE; // required for FETCH processing later on
    }

    if ( networkPolicy() != NETWORK_OFFLINE ) {
        findTaskResponsibleFor( mailboxPtr );
        // and that's all -- the task will detect following replies and sync automatically
    }
}

void Model::_askForNumberOfMessages( TreeItemMsgList* item )
{
    Q_ASSERT( item->parent() );
    TreeItemMailbox* mailboxPtr = dynamic_cast<TreeItemMailbox*>( item->parent() );
    Q_ASSERT( mailboxPtr );

    if ( networkPolicy() == NETWORK_OFFLINE ) {
        Imap::Mailbox::SyncState syncState = cache()->mailboxSyncState( mailboxPtr->mailbox() );
        if ( syncState.isUsableForNumbers() ) {
            item->_unreadMessageCount = syncState.unSeenCount();
            item->_totalMessageCount = syncState.exists();
            item->_recentMessageCount = syncState.recent();
            item->_numberFetchingStatus = TreeItem::DONE;
            emitMessageCountChanged( mailboxPtr );
        } else {
            item->_numberFetchingStatus = TreeItem::UNAVAILABLE;
        }
    } else {
        _taskFactory->createNumberOfMessagesTask(this, mailboxPtr->toIndex(this));
    }
}

void Model::_askForMsgMetadata( TreeItemMessage* item )
{
    Q_ASSERT(item->uid());
    Q_ASSERT(!item->fetched());
    TreeItemMsgList* list = dynamic_cast<TreeItemMsgList*>( item->parent() );
    Q_ASSERT( list );
    TreeItemMailbox* mailboxPtr = dynamic_cast<TreeItemMailbox*>( list->parent() );
    Q_ASSERT( mailboxPtr );

    if ( item->uid() ) {
        AbstractCache::MessageDataBundle data = cache()->messageMetadata( mailboxPtr->mailbox(), item->uid() );
        if ( data.uid == item->uid() ) {
            item->_envelope = data.envelope;
            item->_flags = cache()->msgFlags( mailboxPtr->mailbox(), item->uid() );
            item->_size = data.size;
            QDataStream stream( &data.serializedBodyStructure, QIODevice::ReadOnly );
            stream.setVersion(QDataStream::Qt_4_6);
            QVariantList unserialized;
            stream >> unserialized;
            QSharedPointer<Message::AbstractMessage> abstractMessage;
            try {
                abstractMessage = Message::AbstractMessage::fromList( unserialized, QByteArray(), 0 );
            } catch ( Imap::ParserException& e ) {
                qDebug() << "Error when parsing cached BODYSTRUCTURE" << e.what();
            }
            if ( ! abstractMessage ) {
                item->_fetchStatus = TreeItem::UNAVAILABLE;
            } else {
                QList<TreeItem*> newChildren = abstractMessage->createTreeItems( item );
                if ( item->_children.isEmpty() ) {
                    QList<TreeItem*> oldChildren = item->setChildren( newChildren );
                    Q_ASSERT( oldChildren.size() == 0 );
                } else {
                    QModelIndex messageIdx = item->toIndex(this);
                    // The following assert guards against that crazy signal emitting we had when various _askFor*()
                    // functions were not delayed. If it gets hit, it means that someone tried to call this function
                    // on an item which was already loaded.
                    Q_ASSERT(item->_children.isEmpty());
                    item->setChildren(newChildren);
                }
                item->_fetchStatus = TreeItem::DONE;
            }
        }
    }

    if ( item->fetched() ) {
        // Nothing to do here
        return;
    }

    int order = item->row();

    switch ( networkPolicy() ) {
        case NETWORK_OFFLINE:
            if ( item->_fetchStatus != TreeItem::DONE )
                item->_fetchStatus = TreeItem::UNAVAILABLE;
            break;
        case NETWORK_EXPENSIVE:
            item->_fetchStatus = TreeItem::LOADING;
            findTaskResponsibleFor(mailboxPtr)->requestEnvelopeDownload(item->uid());
            break;
        case NETWORK_ONLINE:
            {
                // preload
                bool ok;
                int preload = property( "trojita-imap-preload-msg-metadata" ).toInt( &ok );
                if ( ! ok )
                    preload = 50;
                KeepMailboxOpenTask *keepTask = findTaskResponsibleFor(mailboxPtr);
                for ( int i = qMax( 0, order - preload ); i < qMin( list->_children.size(), order + preload ); ++i ) {
                    TreeItemMessage* message = dynamic_cast<TreeItemMessage*>( list->_children[i] );
                    Q_ASSERT( message );
                    if ( item == message || ( ! message->fetched() && ! message->loading() && message->uid() ) ) {
                        message->_fetchStatus = TreeItem::LOADING;
                        keepTask->requestEnvelopeDownload(message->uid());
                    }
                }
            }
            break;
    }
}

void Model::_askForMsgPart( TreeItemPart* item, bool onlyFromCache )
{
    // FIXME: fetch parts in chunks, not at once
    Q_ASSERT( item->message() ); // TreeItemMessage
    Q_ASSERT( item->message()->parent() ); // TreeItemMsgList
    Q_ASSERT( item->message()->parent()->parent() ); // TreeItemMailbox
    TreeItemMailbox* mailboxPtr = dynamic_cast<TreeItemMailbox*>( item->message()->parent()->parent() );
    Q_ASSERT( mailboxPtr );

    // We are asking for a message part, which means that the structure of a message is already known.
    // If the UID was zero at this point, it would mean that we are completely doomed.
    uint uid = static_cast<TreeItemMessage*>( item->message() )->uid();
    Q_ASSERT(uid);

    const QByteArray& data = cache()->messagePart( mailboxPtr->mailbox(), uid, item->partId() );
    if ( ! data.isNull() ) {
        item->_data = data;
        item->_fetchStatus = TreeItem::DONE;
    }

    if ( networkPolicy() == NETWORK_OFFLINE ) {
        if ( item->_fetchStatus != TreeItem::DONE )
            item->_fetchStatus = TreeItem::UNAVAILABLE;
    } else if ( ! onlyFromCache ) {
        findTaskResponsibleFor( mailboxPtr )->requestPartDownload( item->message()->_uid, item->partIdForFetch(), item->octets() );
    }
}

void Model::resyncMailbox( const QModelIndex& mbox )
{
    findTaskResponsibleFor( mbox )->resynchronizeMailbox();
}

void Model::setNetworkPolicy( const NetworkPolicy policy )
{
    // If we're connecting after being offline, we should ask for an updated list of mailboxes
    // The main reason is that this happens after entering wrong password and going back online
    bool shouldReloadMailboxes = _netPolicy == NETWORK_OFFLINE && policy != NETWORK_OFFLINE;
    switch ( policy ) {
        case NETWORK_OFFLINE:
            for ( QMap<Parser*,ParserState>::iterator it = _parsers.begin(); it != _parsers.end(); ++it ) {
                Q_ASSERT( it->parser );
                if ( it->maintainingTask ) {
                    it->maintainingTask->stopForLogout();
                }
                it->logoutCmd = it->parser->logout();
                it->connState = CONN_STATE_LOGOUT;
                emit activityHappening( true );
            }
            emit networkPolicyOffline();
            _netPolicy = NETWORK_OFFLINE;
            // FIXME: kill the connection
            break;
        case NETWORK_EXPENSIVE:
            _netPolicy = NETWORK_EXPENSIVE;
            emit networkPolicyExpensive();
            break;
        case NETWORK_ONLINE:
            _netPolicy = NETWORK_ONLINE;
            emit networkPolicyOnline();
            break;
    }
    if ( shouldReloadMailboxes )
        reloadMailboxList();
}

void Model::slotParserDisconnected(Imap::Parser *parser, const QString msg)
{
    emit connectionError(msg);

    if (!parser)
        return;

    // This function is *not* called from inside the responseReceived(), so we have to remove the parser from the list, too
    killParser(parser, PARSER_KILL_EXPECTED);
    _parsers.remove(parser);
    parsersMightBeIdling();
}

void Model::broadcastParseError( const uint parser, const QString& exceptionClass, const QString& errorMessage, const QByteArray& line, int position )
{
    emit logParserFatalError( parser, exceptionClass, errorMessage, line, position );
    QByteArray details = ( position == -1 ) ? QByteArray() : QByteArray( position, ' ' ) + QByteArray("^ here");
    emit connectionError( trUtf8( "<p>The IMAP server sent us a reply which we could not parse. "
                                  "This might either mean that there's a bug in Trojiá's code, or "
                                  "that the IMAP server you are connected to is broken. Please "
                                  "report this as a bug anyway. Here are the details:</p>"
                                  "<p><b>%1</b>: %2</p>"
                                  "<pre>%3\n%4</pre>"
                                  ).arg( exceptionClass, errorMessage, line, details ) );
}

void Model::slotParseError(Parser *parser, const QString &exceptionClass, const QString &errorMessage, const QByteArray &line, int position)
{
    Q_ASSERT(parser);

    broadcastParseError(parser->parserId(), exceptionClass, errorMessage, line, position);

    // This function is *not* called from inside the responseReceived(), so we have to remove the parser from the list, too
    killParser(parser, PARSER_KILL_HARD);
    _parsers.remove(parser);
    parsersMightBeIdling();
}

void Model::switchToMailbox( const QModelIndex& mbox )
{
    if ( ! mbox.isValid() )
        return;

    if ( _netPolicy == NETWORK_OFFLINE )
        return;

    findTaskResponsibleFor( mbox );
}

void Model::updateCapabilities( Parser* parser, const QStringList capabilities )
{
    QStringList uppercaseCaps;
    Q_FOREACH( const QString& str, capabilities )
            uppercaseCaps << str.toUpper();
    accessParser( parser ).capabilities = uppercaseCaps;
    accessParser( parser ).capabilitiesFresh = true;
    parser->enableLiteralPlus( uppercaseCaps.contains( QLatin1String( "LITERAL+" ) ) );
    if ( _parsers.begin().key() == parser )
        emit capabilitiesUpdated(uppercaseCaps);
}

void Model::markMessagesDeleted(const QModelIndexList &messages, bool marked)
{
    Q_ASSERT(!messages.isEmpty());
    Q_ASSERT(messages.front().model() == this);
    _taskFactory->createUpdateFlagsTask( this, messages,
                         marked ? QLatin1String("+FLAGS") : QLatin1String("-FLAGS"),
                         QLatin1String("(\\Deleted)") );
}

void Model::markMessagesRead(const QModelIndexList &messages, bool marked)
{
    Q_ASSERT(!messages.isEmpty());
    Q_ASSERT(messages.front().model() == this);
    _taskFactory->createUpdateFlagsTask( this, messages,
                         marked ? QLatin1String("+FLAGS") : QLatin1String("-FLAGS"),
                         QLatin1String("(\\Seen)") );
}

void Model::copyMoveMessages( TreeItemMailbox* sourceMbox, const QString& destMailboxName, QList<uint> uids, const CopyMoveOperation op )
{
    if ( _netPolicy == NETWORK_OFFLINE ) {
        // FIXME: error signalling
        return;
    }

    Q_ASSERT( sourceMbox );

    qSort( uids );

    QModelIndexList messages;
    Sequence seq;
    Q_FOREACH( TreeItemMessage* m, findMessagesByUids( sourceMbox, uids ) ) {
        messages << m->toIndex(this);
        seq.add( m->uid() );
    }
    _taskFactory->createCopyMoveMessagesTask( this, messages, destMailboxName, op );
}

/** @short Convert a list of UIDs to a list of pointers to the relevant message nodes */
QList<TreeItemMessage*> Model::findMessagesByUids(const TreeItemMailbox* const mailbox, const QList<uint> &uids)
{
    const TreeItemMsgList* const list = dynamic_cast<const TreeItemMsgList* const>(mailbox->_children[0]);
    Q_ASSERT(list);
    QList<TreeItemMessage*> res;
    QList<TreeItem*>::const_iterator it = list->_children.constBegin();
    // qBinaryFind is not designed to operate on a value of a different kind than stored in the container
    // so we can't really compare TreeItem* with uint, even though our LessThan supports that
    // (it keeps calling it both via LowerThan(*it, value) and LowerThan(value, *it).
    TreeItemMessage fakeMessage(0);
    uint lastUid = 0;
    Q_FOREACH(const uint& uid, uids) {
        if (lastUid == uid) {
            // we have to filter out duplicates
            continue;
        }
        lastUid = uid;
        fakeMessage._uid = uid;
        it = qBinaryFind(it, list->_children.constEnd(), &fakeMessage, uidComparator);
        if (it != list->_children.end()) {
            res << static_cast<TreeItemMessage*>(*it);
        } else {
            qDebug() << "Can't find UID" << uid;
        }
    }
    return res;
}

TreeItemMailbox* Model::findMailboxByName(const QString& name) const
{
    return findMailboxByName(name, _mailboxes);
}

TreeItemMailbox* Model::findMailboxByName(const QString& name, const TreeItemMailbox* const root) const
{
    Q_ASSERT(!root->_children.isEmpty());
    // Names are sorted, so linear search is not required. On the ohterhand, the mailbox sizes are typically small enough
    // so that this shouldn't matter at all, and linear search is simple enough.
    for (int i = 1; i < root->_children.size(); ++i) {
        TreeItemMailbox* mailbox = static_cast<TreeItemMailbox*>(root->_children[i]);
        if (name == mailbox->mailbox())
            return mailbox;
        else if (name.startsWith(mailbox->mailbox() + mailbox->separator()))
            return findMailboxByName(name, mailbox);
    }
    return 0;
}

/** @short Find a parent mailbox for the specified name */
TreeItemMailbox* Model::findParentMailboxByName(const QString& name) const
{
    TreeItemMailbox* root = _mailboxes;
    while (true) {
        if (root->_children.size() == 1) {
            break;
        }
        bool found = false;
        for (int i = 1; !found && i < root->_children.size(); ++i) {
            TreeItemMailbox* const item = dynamic_cast<TreeItemMailbox*>(root->_children[i]);
            Q_ASSERT(item);
            if (name.startsWith(item->mailbox() + item->separator())) {
                root = item;
                found = true;
            }
        }
        if (!found) {
            return root;
        }
    }
    return root;
}


void Model::expungeMailbox(TreeItemMailbox* mbox)
{
    if (!mbox)
        return;

    if (_netPolicy == NETWORK_OFFLINE) {
        qDebug() << "Can't expunge while offline";
        return;
    }

    _taskFactory->createExpungeMailboxTask(this, mbox->toIndex(this));
}

void Model::createMailbox(const QString& name)
{
    if (_netPolicy == NETWORK_OFFLINE) {
        qDebug() << "Can't create mailboxes while offline";
        return;
    }

    _taskFactory->createCreateMailboxTask(this, name);
}

void Model::deleteMailbox( const QString& name )
{
    if ( _netPolicy == NETWORK_OFFLINE ) {
        qDebug() << "Can't delete mailboxes while offline";
        return;
    }

    _taskFactory->createDeleteMailboxTask( this, name );
}

void Model::saveUidMap( TreeItemMsgList* list )
{
    QList<uint> seqToUid;
    for ( int i = 0; i < list->_children.size(); ++i )
        seqToUid << static_cast<TreeItemMessage*>( list->_children[ i ] )->uid();
    cache()->setUidMapping( static_cast<TreeItemMailbox*>( list->parent() )->mailbox(), seqToUid );
}


TreeItem* Model::realTreeItem( QModelIndex index, const Model** whichModel, QModelIndex* translatedIndex )
{
    while ( const QAbstractProxyModel* proxy = qobject_cast<const QAbstractProxyModel*>( index.model() ) ) {
        index = proxy->mapToSource( index );
        proxy = qobject_cast<const QAbstractProxyModel*>( index.model() );
    }
    const Model* model = qobject_cast<const Model*>( index.model() );
    Q_ASSERT(model);
    if ( whichModel )
        *whichModel = model;
    if ( translatedIndex )
        *translatedIndex = index;
    return static_cast<TreeItem*>( index.internalPointer() );
}

CommandHandle Model::performAuthentication( Imap::Parser* ptr )
{
    // The LOGINDISABLED capability is checked elsewhere
    if ( ! _authenticator ) {
        _authenticator = new QAuthenticator();
        emit authRequested( _authenticator );
    }

    if ( _authenticator->isNull() ) {
        delete _authenticator;
        _authenticator = 0;
        emit connectionError( tr("Can't login without user/password data") );
        return CommandHandle();
    } else {
        CommandHandle cmd = ptr->login( _authenticator->user(), _authenticator->password() );
        emit activityHappening( true );
        return cmd;
    }
}

void Model::changeConnectionState(Parser *parser, ConnectionState state)
{
    accessParser( parser ).connState = state;
    emit connectionStateChanged( parser, state );
}

void Model::handleSocketStateChanged( Parser *parser, Imap::ConnectionState state)
{
    Q_ASSERT(parser);
    if ( accessParser( parser ).connState < state ) {
        changeConnectionState( parser, state );
    }
}

void Model::parserIsSendingCommand( Parser *parser, const QString& tag)
{
    Q_ASSERT(parser);
    Q_UNUSED(tag);
    // FIXME: move this somewhere else!
    /*QMap<CommandHandle, CommandKind>::const_iterator it = accessParser( parser ).commandMap.find( tag );
    if ( it == accessParser( parser ).commandMap.end() ) {
        qDebug() << "Dunno anything about command" << tag;
        return;
    }

    switch ( *it ) {
        case CMD_NONE: // invalid
        case CMD_STARTTLS: // handled elsewhere
        case CMD_NAMESPACE: // FIXME: not needed yet
        case CMD_LOGOUT: // not worth the effort
            break;
        case CMD_LOGIN:
            changeConnectionState( parser, CONN_STATE_LOGIN );
            break;
        case CMD_SELECT:
            changeConnectionState( parser, CONN_STATE_SELECTING );
            break;
        case CMD_FETCH_WITH_FLAGS:
            changeConnectionState( parser, CONN_STATE_SYNCING );
            break;
        case CMD_FETCH_PART:
            changeConnectionState( parser, CONN_STATE_FETCHING_PART );
            break;
        case CMD_FETCH_MESSAGE_METADATA:
            changeConnectionState( parser, CONN_STATE_FETCHING_MSG_METADATA );
            break;
        case CMD_NOOP:
        case CMD_IDLE:
            // do nothing
            break;
    }*/
}

void Model::parsersMightBeIdling()
{
    bool someParserBusy = false;
    // FIXME: track activity on a Task basis...
    emit activityHappening( someParserBusy );
}

void Model::killParser(Parser *parser, ParserKillingMethod method)
{
    Q_FOREACH(ImapTask *task, accessParser(parser).activeTasks) {
        task->die();
        task->deleteLater();
    }

    parser->disconnect();
    parser->deleteLater();
    accessParser(parser).parser = 0;
    switch (method) {
    case PARSER_KILL_EXPECTED:
        logTrace(parser->parserId(), LOG_IO_WRITTEN, QString(), "*** Connection closed.");
        return;
    case PARSER_KILL_HARD:
        logTrace(parser->parserId(), LOG_IO_WRITTEN, QString(), "*** Connection killed.");
        return;
    }
    Q_ASSERT(false);
}

void Model::slotParserLineReceived( Parser *parser, const QByteArray& line )
{
    logTrace(parser->parserId(), LOG_IO_READ, QString(), line);
}

void Model::slotParserLineSent( Parser *parser, const QByteArray& line )
{
    logTrace(parser->parserId(), LOG_IO_WRITTEN, QString(), line);
}

void Model::setCache( AbstractCache* cache )
{
    if ( _cache )
        _cache->deleteLater();
    _cache = cache;
    _cache->setParent( this );
}

void Model::runReadyTasks()
{
    for ( QMap<Parser*,ParserState>::iterator parserIt = _parsers.begin(); parserIt != _parsers.end(); ++parserIt ) {
        bool runSomething = false;
        do {
            runSomething = false;
            // See responseReceived() for more details about why we do need to iterate over a copy here.
            // Basically, calls to ImapTask::perform could invalidate our precious iterators.
            QList<ImapTask*> origList = parserIt->activeTasks;
            QList<ImapTask*> deletedList;
            QList<ImapTask*>::const_iterator taskEnd = origList.constEnd();
            for ( QList<ImapTask*>::const_iterator taskIt = origList.constBegin(); taskIt != taskEnd; ++taskIt ) {
                ImapTask *task = *taskIt;
                if ( task->isReadyToRun() ) {
                    task->perform();
                    runSomething = true;
                }
                if ( task->isFinished() ) {
                    deletedList << task;
                }
            }
            removeDeletedTasks( deletedList, parserIt->activeTasks );
        } while ( runSomething );
    }
}

void Model::removeDeletedTasks( const QList<ImapTask*>& deletedTasks, QList<ImapTask*>& activeTasks )
{
    // Remove the finished commands
    for ( QList<ImapTask*>::const_iterator deletedIt = deletedTasks.begin(); deletedIt != deletedTasks.end(); ++deletedIt ) {
        (*deletedIt)->deleteLater();
        activeTasks.removeOne( *deletedIt );
    }
}

KeepMailboxOpenTask* Model::findTaskResponsibleFor( const QModelIndex& mailbox )
{
    Q_ASSERT( mailbox.isValid() );
    QModelIndex translatedIndex;
    TreeItemMailbox* mailboxPtr = dynamic_cast<TreeItemMailbox*>( realTreeItem( mailbox, 0, &translatedIndex ) );
    return findTaskResponsibleFor( mailboxPtr );
}

KeepMailboxOpenTask* Model::findTaskResponsibleFor(TreeItemMailbox *mailboxPtr)
{
    Q_ASSERT( mailboxPtr );
    bool canCreateParallelConn = _parsers.isEmpty(); // FIXME: multiple connections

    if (mailboxPtr->maintainingTask) {
        // The requested mailbox already has the maintaining task associated
        if (accessParser(mailboxPtr->maintainingTask->parser).connState == CONN_STATE_LOGOUT) {
            // The connection is currently getting closed, so we have to create another one
            return _taskFactory->createKeepMailboxOpenTask(this, mailboxPtr->toIndex(this), 0);
        } else {
            // it's usable as-is
            return mailboxPtr->maintainingTask;
        }
    } else if (canCreateParallelConn) {
        // The mailbox is not being maintained, but we can create a new connection
        return _taskFactory->createKeepMailboxOpenTask(this, mailboxPtr->toIndex(this), 0);
    } else {
        // Too bad, we have to re-use an existing parser. That will probably lead to
        // stealing it from some mailbox, but there's no other way.
        Q_ASSERT(!_parsers.isEmpty());

        if (_parsers.begin()->connState == CONN_STATE_LOGOUT) {
            // At this point, we have no other choice than create a nwe connection
            return _taskFactory->createKeepMailboxOpenTask(this, mailboxPtr->toIndex(this), 0);
        } else {
            return _taskFactory->createKeepMailboxOpenTask(this, mailboxPtr->toIndex(this), _parsers.begin().key());
        }
    }



}
void Model::_genericHandleFetch( TreeItemMailbox* mailbox, const Imap::Responses::Fetch* const resp )
{
    Q_ASSERT(mailbox);
    QList<TreeItemPart*> changedParts;
    TreeItemMessage* changedMessage = 0;
    mailbox->handleFetchResponse( this, *resp, changedParts, changedMessage );
    if ( ! changedParts.isEmpty() ) {
        Q_FOREACH( TreeItemPart* part, changedParts ) {
            QModelIndex index = part->toIndex(this);
            emit dataChanged( index, index );
        }
    }
    if (changedMessage) {
        QModelIndex index = changedMessage->toIndex(this);
        emit dataChanged(index, index);
        emitMessageCountChanged(mailbox);
    }
}

QModelIndex Model::findMailboxForItems( const QModelIndexList& items )
{
    TreeItemMailbox* mailbox = 0;
    Q_FOREACH( const QModelIndex& index, items ) {
        TreeItemMailbox* currentMailbox = 0;
        Q_ASSERT(index.model() == this);

        TreeItem* item = static_cast<TreeItem*>( index.internalPointer() );
        Q_ASSERT(item);

        TreeItemMessage* message = dynamic_cast<TreeItemMessage*>( item );
        if ( ! message ) {
            if ( TreeItemPart* part = dynamic_cast<TreeItemPart*>( item ) ) {
                message = part->message();
            } else {
                throw CantHappen( "findMailboxForItems() called on strange items");
            }
        }
        Q_ASSERT(message);
        TreeItemMsgList* list = dynamic_cast<TreeItemMsgList*>( message->parent() );
        Q_ASSERT(list);
        currentMailbox = dynamic_cast<TreeItemMailbox*>( list->parent() );
        Q_ASSERT(currentMailbox);
        if ( ! mailbox ) {
            mailbox = currentMailbox;
        } else if ( mailbox != currentMailbox ) {
            throw CantHappen( "Messages from several mailboxes");
        }
    }
    return mailbox->toIndex(this);
}

void Model::slotTasksChanged()
{
    QList<ImapTask*> tasks = findChildren<ImapTask*>();
    qDebug() << "-------------";
    Q_FOREACH( ParserState parserState, _parsers ) {
        qDebug() << "Parser" << parserState.parser->parserId() << parserState.activeTasks.size() << "active tasks";
    }
    int i = 0;
    Q_FOREACH( ImapTask* task, tasks ) {
        QString finished = ( task->isFinished() ? "[finished] " : "" );
        QString isReadyToRun = ( task->isReadyToRun() ? "[ready-to-run] " : "" );
        QString isActive;
        Q_FOREACH( ParserState parserState, _parsers ) {
            if ( parserState.activeTasks.contains( task ) ) {
                isActive = "[active] ";
                break;
            }
        }
        qDebug() << task << task->debugIdentification() << finished << isReadyToRun << isActive; // << task->dependentTasks;
        ++i;
        if ( i > 50 ) {
            qDebug() << "...and" << tasks.size() - i << "more tasks.";
            break;
        }
    }
    qDebug() << "-------------";
}

void Model::slotTaskDying( QObject *obj )
{
    ImapTask *task = static_cast<ImapTask*>( obj );
    for ( QMap<Parser*,ParserState>::iterator it = _parsers.begin(); it != _parsers.end(); ++it ) {
        it->activeTasks.removeOne( task );
    }
}

TreeItemMailbox* Model::mailboxForSomeItem( QModelIndex index )
{
    TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( index.internalPointer() ) );
    while ( index.isValid() && ! mailbox ) {
        index = index.parent();
        mailbox = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( index.internalPointer() ) );
    }
    return mailbox;
}

Model::ParserState& Model::accessParser( Parser *parser )
{
    Q_ASSERT( _parsers.contains( parser ) );
    return _parsers[ parser ];
}

QModelIndex Model::findMessageForItem( QModelIndex index )
{
    if ( ! index.isValid() )
        return QModelIndex();

    if ( ! dynamic_cast<const Model*>( index.model() ) )
        return QModelIndex();

    TreeItem* item = static_cast<TreeItem*>( index.internalPointer() );
    Q_ASSERT( item );
    while ( item ) {
        Q_ASSERT( index.internalPointer() == item );
        if ( dynamic_cast<TreeItemMessage*>(item) ) {
            return index;
        } else if ( dynamic_cast<TreeItemPart*>(item) ) {
            index = index.parent();
            item = item->parent();
        } else {
            return QModelIndex();
        }
    }
    return QModelIndex();
}

void Model::releaseMessageData( const QModelIndex &message )
{
    if ( ! message.isValid() )
        return;

    const Model *whichModel = 0;
    QModelIndex realMessage;
    realTreeItem( message, &whichModel, &realMessage );
    Q_ASSERT( whichModel == this );

    TreeItemMessage *msg = dynamic_cast<TreeItemMessage*>( static_cast<TreeItem*>( realMessage.internalPointer() ) );
    if ( ! msg )
        return;

    msg->_fetchStatus = TreeItem::NONE;
    msg->_envelope.clear();
    msg->_partHeader->silentlyReleaseMemoryRecursive();
    msg->_partText->silentlyReleaseMemoryRecursive();
    Q_FOREACH( TreeItem *item, msg->_children ) {
        TreeItemPart *part = dynamic_cast<TreeItemPart*>( item );
        Q_ASSERT(part);
        part->silentlyReleaseMemoryRecursive();
    }
    emit dataChanged( realMessage, realMessage );
}

QStringList Model::capabilities() const
{
    if ( _parsers.isEmpty() )
        return QStringList();

    if ( _parsers.constBegin()->capabilitiesFresh )
        return _parsers.constBegin()->capabilities;

    return QStringList();
}

void Model::emitAuthFailed(const QString &message)
{
    delete _authenticator;
    _authenticator = 0;
    emit authAttemptFailed(message);
}

void Model::logTrace(uint parserId, const LogKind kind, const QString &source, const QString &message)
{
    enum {CUTOFF=200};
    uint truncatedBytes = message.size() > CUTOFF ? message.size() - CUTOFF : 0;
    LogMessage m(QDateTime::currentDateTime(), kind, source, truncatedBytes ? message.left(CUTOFF) : message, truncatedBytes);
    emit logged(parserId, m);
}

}
}
