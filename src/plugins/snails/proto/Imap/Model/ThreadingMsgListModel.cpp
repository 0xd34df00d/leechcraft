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

#include "ThreadingMsgListModel.h"
#include <algorithm>
#include <QDebug>
#include "ItemRoles.h"
#include "MailboxTree.h"
#include "MsgListModel.h"

namespace Imap {
namespace Mailbox {

ThreadingMsgListModel::ThreadingMsgListModel( QObject* parent ): QAbstractProxyModel(parent), modelResetInProgress(false)
{
}

void ThreadingMsgListModel::setSourceModel( QAbstractItemModel *sourceModel )
{
    _threading.clear();
    ptrToInternal.clear();
    unknownUids.clear();

    if (this->sourceModel()) {
        // there's already something, so take care to disconnect all signals
        this->sourceModel()->disconnect(this);
    }

    reset();
    Imap::Mailbox::MsgListModel *msgList = qobject_cast<Imap::Mailbox::MsgListModel*>( sourceModel );
    QAbstractProxyModel::setSourceModel( msgList );
    if ( ! msgList )
        return;

    // FIXME: will need to be expanded when Model supports more signals...
    connect( sourceModel, SIGNAL( modelReset() ), this, SLOT( resetMe() ) );
    connect( sourceModel, SIGNAL( layoutAboutToBeChanged() ), this, SIGNAL( layoutAboutToBeChanged() ) );
    connect( sourceModel, SIGNAL( layoutChanged() ), this, SIGNAL( layoutChanged() ) );
    connect( sourceModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
            this, SLOT( handleDataChanged( const QModelIndex&, const QModelIndex& ) ) );
    connect( sourceModel, SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsAboutToBeRemoved(const QModelIndex&, int,int ) ) );
    connect( sourceModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsRemoved(const QModelIndex&, int,int ) ) );
    connect( sourceModel, SIGNAL( rowsAboutToBeInserted( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsAboutToBeInserted(const QModelIndex&, int,int ) ) );
    connect( sourceModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsInserted(const QModelIndex&, int,int ) ) );
    resetMe();
}

void ThreadingMsgListModel::handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    if ( topLeft.row() != bottomRight.row() ) {
        // FIXME: Batched updates...
        Q_ASSERT(false);
        return;
    }

    if ( unknownUids.contains(topLeft) ) {
        // The message wasn't fully synced before, and now it is
        unknownUids.removeOne(topLeft);
        qDebug() << "Got UID for" << topLeft.row();
        if ( unknownUids.isEmpty() ) {
            // Let's re-thread, then!
            askForThreading();
        }
        return;
    }

    QModelIndex first = mapFromSource( topLeft );
    QModelIndex second = mapFromSource( bottomRight );

    // There's a possibility for a short race window when te underlying model signals
    // new data for a message but said message doesn't have threading info yet. Therefore,
    // the translated indexes are invalid, so we have to handle that gracefully.
    // "Doing nothing" could be a reasonable thing.
    if ( ! first.isValid() || ! second.isValid() )
        return;

    emit dataChanged( first, second );
}

QModelIndex ThreadingMsgListModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( parent.isValid() && parent.model() != this ) {
        // foreign model
        return QModelIndex();
    }

    if ( _threading.isEmpty() ) {
        // mapping not available yet
        return QModelIndex();
    }

    if ( row < 0 || column < 0 || column >= MsgListModel::COLUMN_COUNT )
        return QModelIndex();

    if ( parent.isValid() && parent.column() != 0 ) {
        // only the first column should have children
        return QModelIndex();
    }

    uint parentId = parent.isValid() ? parent.internalId() : 0;

    QHash<uint,ThreadNodeInfo>::const_iterator it = _threading.constFind( parentId );
    Q_ASSERT(it != _threading.constEnd());

    if ( it->children.size() <= row )
        return QModelIndex();

    return createIndex( row, column, it->children[row] );
}

QModelIndex ThreadingMsgListModel::parent( const QModelIndex& index ) const
{
    if ( ! index.isValid() || index.model() != this )
        return QModelIndex();

    if ( _threading.isEmpty() )
        return QModelIndex();

    if ( index.row() < 0 || index.column() < 0 || index.column() >= MsgListModel::COLUMN_COUNT )
        return QModelIndex();

    QHash<uint,ThreadNodeInfo>::const_iterator node = _threading.constFind( index.internalId() );
    if ( node == _threading.constEnd() )
        return QModelIndex();

    QHash<uint,ThreadNodeInfo>::const_iterator parentNode = _threading.constFind( node->parent );
    Q_ASSERT(parentNode != _threading.constEnd());
    Q_ASSERT(parentNode->internalId == node->parent);

    if ( parentNode->internalId == 0 )
        return QModelIndex();

    QHash<uint,ThreadNodeInfo>::const_iterator grantParentNode = _threading.constFind( parentNode->parent );
    Q_ASSERT(grantParentNode != _threading.constEnd());
    Q_ASSERT(grantParentNode->internalId == parentNode->parent);

    return createIndex( grantParentNode->children.indexOf( parentNode->internalId ), 0, parentNode->internalId );
}

bool ThreadingMsgListModel::hasChildren( const QModelIndex& parent ) const
{
    if ( parent.isValid() && parent.column() != 0 )
        return false;

    return ! _threading.isEmpty() && ! _threading.value( parent.internalId() ).children.isEmpty();
}

int ThreadingMsgListModel::rowCount( const QModelIndex& parent ) const
{
    if ( _threading.isEmpty() )
        return 0;

    if ( parent.isValid() && parent.column() != 0 )
        return 0;

    return _threading.value( parent.internalId() ).children.size();
}

int ThreadingMsgListModel::columnCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() && parent.column() != 0 )
        return 0;

    return MsgListModel::COLUMN_COUNT;
}

QModelIndex ThreadingMsgListModel::mapToSource( const QModelIndex& proxyIndex ) const
{
    if ( !proxyIndex.isValid() || !proxyIndex.internalId() )
        return QModelIndex();

    if ( _threading.isEmpty() )
        return QModelIndex();

    Imap::Mailbox::MsgListModel *msgList = qobject_cast<Imap::Mailbox::MsgListModel*>( sourceModel() );
    Q_ASSERT(msgList);

    QHash<uint,ThreadNodeInfo>::const_iterator node = _threading.constFind( proxyIndex.internalId() );
    if ( node == _threading.constEnd() )
        return QModelIndex();

    if (node->ptr) {
        return msgList->createIndex( node->ptr->row(), proxyIndex.column(), node->ptr );
    } else {
        // it's a fake message
        return QModelIndex();
    }
}

QModelIndex ThreadingMsgListModel::mapFromSource( const QModelIndex& sourceIndex ) const
{
    if ( sourceIndex.model() != sourceModel() )
        return QModelIndex();

    QHash<void*,uint>::const_iterator it = ptrToInternal.constFind( sourceIndex.internalPointer() );
    if ( it == ptrToInternal.constEnd() )
        return QModelIndex();

    const uint internalId = *it;

    QHash<uint,ThreadNodeInfo>::const_iterator node = _threading.constFind( internalId );
    Q_ASSERT(node != _threading.constEnd());

    QHash<uint,ThreadNodeInfo>::const_iterator parentNode = _threading.constFind( node->parent );
    Q_ASSERT(parentNode != _threading.constEnd());
    int offset = parentNode->children.indexOf( internalId );
    Q_ASSERT( offset != -1 );

    return createIndex( offset, sourceIndex.column(), internalId );
}

QVariant ThreadingMsgListModel::data( const QModelIndex &proxyIndex, int role ) const
{
    if ( ! proxyIndex.isValid() || proxyIndex.model() != this )
        return QVariant();

    QHash<uint,ThreadNodeInfo>::const_iterator it = _threading.constFind( proxyIndex.internalId() );
    Q_ASSERT(it != _threading.constEnd());

    if (it->ptr) {
        // It's a real item which exists in the underlying model
        if ( role == RoleThreadRootWithUnreadMessages && ! proxyIndex.parent().isValid() ) {
            return threadContainsUnreadMessages(it->internalId);
        } else {
            return QAbstractProxyModel::data( proxyIndex, role );
        }
    }

    switch( role ) {
    case Qt::DisplayRole:
        if ( proxyIndex.column() == 0 )
            return tr("[Message is missing]");
        break;
    case Qt::ToolTipRole:
        return tr("This thread refers to an extra message, but that message is not present in the "
                  "selected mailbox, or is missing from the current search context.");
    }
    return QVariant();
}

Qt::ItemFlags ThreadingMsgListModel::flags( const QModelIndex &index ) const
{
    if ( ! index.isValid() || index.model() != this )
        return Qt::NoItemFlags;

    QHash<uint,ThreadNodeInfo>::const_iterator it = _threading.constFind( index.internalId() );
    Q_ASSERT(it != _threading.constEnd());
    if (it->ptr)
        return QAbstractProxyModel::flags( index );

    return Qt::NoItemFlags;

}

void ThreadingMsgListModel::handleRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());

    for (int i = start; i <= end; ++i) {
        QModelIndex index = sourceModel()->index(i, 0, parent);
        Q_ASSERT(index.isValid());
        uint uid = index.data(Imap::Mailbox::RoleMessageUid).toUInt();
        if (uid) {
            // Removing a message with an already known UID. We'll just mark it for deletion.
            QModelIndex translated = mapFromSource(index);
            Q_ASSERT(translated.isValid());
            QHash<uint,ThreadNodeInfo>::iterator it = _threading.find(translated.internalId());
            Q_ASSERT(it != _threading.end());
            it->uid = 0;
            it->ptr = 0;
            // it will get cleaned up by the pruneTree call later on
        } else {
            // removing message without a UID
            unknownUids.removeOne(index);
            // such a message is not in the mapping yet, and therefore invisible
        }
    }
}

void ThreadingMsgListModel::handleRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());

    Q_UNUSED(start);
    Q_UNUSED(end);

    // It looks like this simplified approach won't really fly when model starts to issue interleaved rowsRemoved signals,
    // as we'll just remove everything upon first rowsRemoved.  I'll just hope that it doesn't matter (much).

    emit layoutAboutToBeChanged();
    pruneTree();
    updatePersistentIndexesPhase1();
    updatePersistentIndexesPhase2();
    emit layoutChanged();
}

void ThreadingMsgListModel::handleRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());

    int myStart = _threading[0].children.size();
    int myEnd = myStart + (end - start);
    beginInsertRows(QModelIndex(), myStart, myEnd);
}

void ThreadingMsgListModel::handleRowsInserted( const QModelIndex& parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());

    for (int i = start; i <= end; ++i) {
        QModelIndex index = sourceModel()->index(i, 0);
        uint uid = index.data(RoleMessageUid).toUInt();
        ThreadNodeInfo node;
        node.internalId = i + 1;
        node.uid = uid;
        node.ptr = static_cast<TreeItem*>(index.internalPointer());
        _threading[node.internalId] = node;
        _threading[0].children << node.internalId;
        ptrToInternal[node.ptr] = node.internalId;
        if (!node.uid) {
            qDebug() << "Message" << index.row() << "has unkown UID";
            unknownUids << index;
        }
    }
    endInsertRows();

    askForThreading();
}

void ThreadingMsgListModel::resetMe()
{
    // Prevent possible recursion here
    if ( modelResetInProgress )
        return;

    modelResetInProgress = true;
    _threading.clear();
    ptrToInternal.clear();
    unknownUids.clear();
    reset();
    updateNoThreading();
    modelResetInProgress = false;

    // If there are any messages, try to thread them
    if ( sourceModel() && rowCount() )
        askForThreading();
}

void ThreadingMsgListModel::updateNoThreading()
{
    if ( ! _threading.isEmpty() ) {
        beginRemoveRows( QModelIndex(), 0, rowCount() - 1 );
        _threading.clear();
        ptrToInternal.clear();
        endRemoveRows();
    }
    unknownUids.clear();

    if ( ! sourceModel() ) {
        // Maybe we got reset because the parent model is no longer here...
        return;
    }

    int upstreamMessages = sourceModel()->rowCount();
    QList<uint> allIds;
    QHash<uint,ThreadNodeInfo> newThreading;
    QHash<void*,uint> newPtrToInternal;

    for ( int i = 0; i < upstreamMessages; ++i ) {
        QModelIndex index = sourceModel()->index( i, 0 );
        uint uid = index.data( RoleMessageUid ).toUInt();
        ThreadNodeInfo node;
        node.internalId = i + 1;
        node.uid = uid;
        node.ptr = static_cast<TreeItem*>( index.internalPointer() );
        newThreading[node.internalId] = node;
        allIds.append(node.internalId);
        newPtrToInternal[node.ptr] = node.internalId;
        if (!node.uid) {
            qDebug() << "Message" << index.row() << "has unkown UID";
            unknownUids << index;
        }
    }

    if ( newThreading.size() ) {
        beginInsertRows( QModelIndex(), 0, newThreading.size() - 1 );
        _threading = newThreading;
        ptrToInternal = newPtrToInternal;
        _threading[ 0 ].children = allIds;
        _threading[ 0 ].ptr = static_cast<MsgListModel*>( sourceModel() )->msgList;
        endInsertRows();
    }
}

void ThreadingMsgListModel::askForThreading()
{
    if ( ! sourceModel() ) {
        updateNoThreading();
        return;
    }

    if ( ! sourceModel()->rowCount() )
        return;

    const Imap::Mailbox::Model *realModel;
    QModelIndex someMessage = sourceModel()->index(0,0);
    QModelIndex realIndex;
    Imap::Mailbox::Model::realTreeItem( someMessage, &realModel, &realIndex );
    QModelIndex mailboxIndex = realIndex.parent().parent();

    if ( realModel->capabilities().contains( QLatin1String("THREAD=REFS")) ) {
        requestedAlgorithm = QLatin1String("REFS");
    } else if ( realModel->capabilities().contains( QLatin1String("THREAD=REFERENCES") ) ) {
        requestedAlgorithm = QLatin1String("REFERENCES");
    } else if ( realModel->capabilities().contains( QLatin1String("THREAD=ORDEREDSUBJECT") ) ) {
        requestedAlgorithm = QLatin1String("ORDEREDSUBJECT");
    }

    if ( ! requestedAlgorithm.isEmpty() ) {
        realModel->_taskFactory->createThreadTask( const_cast<Imap::Mailbox::Model*>(realModel),
                                                   mailboxIndex, requestedAlgorithm,
                                                   QStringList() << QLatin1String("ALL") );
        connect( realModel, SIGNAL(threadingAvailable(QModelIndex,QString,QStringList,QVector<Imap::Responses::ThreadingNode>)),
                 this, SLOT(slotThreadingAvailable(QModelIndex,QString,QStringList,QVector<Imap::Responses::ThreadingNode>)) );
        connect( realModel, SIGNAL(threadingFailed(QModelIndex,QString,QStringList)), this, SLOT(slotThreadingFailed(QModelIndex,QString,QStringList)));
    }
}

bool ThreadingMsgListModel::shouldIgnoreThisThreadingResponse(const QModelIndex &mailbox, const QString &algorithm,
                                                              const QStringList &searchCriteria, const Model **realModel)
{
    QModelIndex someMessage = sourceModel()->index(0,0);
    if (!someMessage.isValid())
        return true;
    const Model *model;
    QModelIndex realIndex;
    Imap::Mailbox::Model::realTreeItem( someMessage, &model, &realIndex );
    QModelIndex mailboxIndex = realIndex.parent().parent();
    if ( mailboxIndex != mailbox ) {
        // this is for another mailbox
        return true;
    }

    if ( algorithm != requestedAlgorithm ) {
        qDebug() << "Weird, asked for threading via" << requestedAlgorithm << " but got" << algorithm <<
                "instead -- ignoring.";
        return true;
    }

    if ( searchCriteria.size() != 1 || searchCriteria.front() != QLatin1String("ALL") ) {
        qDebug() << "Weird, requesting messages matching ALL, but got this instead: " << searchCriteria;
        return true;
    }

    if (realModel)
        *realModel = model;
    return false;
}

void ThreadingMsgListModel::slotThreadingFailed(const QModelIndex &mailbox, const QString &algorithm, const QStringList &searchCriteria)
{
    if ( shouldIgnoreThisThreadingResponse(mailbox, algorithm, searchCriteria) )
        return;

    disconnect( sender(), 0, this,
                SLOT(slotThreadingAvailable(QModelIndex,QString,QStringList,QVector<Imap::Responses::ThreadingNode>)) );
    disconnect( sender(), 0, this,
                SLOT(slotThreadingFailed(QModelIndex,QString,QStringList)) );

    updateNoThreading();
}

void ThreadingMsgListModel::slotThreadingAvailable( const QModelIndex &mailbox, const QString &algorithm,
                                                    const QStringList &searchCriteria,
                                                    const QVector<Imap::Responses::ThreadingNode> &mapping )
{
    const Model *model = 0;
    if ( shouldIgnoreThisThreadingResponse(mailbox, algorithm, searchCriteria, &model) )
        return;

    disconnect( sender(), 0, this,
                SLOT(slotThreadingAvailable(QModelIndex,QString,QStringList,QVector<Imap::Responses::ThreadingNode>)) );
    disconnect( sender(), 0, this,
                SLOT(slotThreadingFailed(QModelIndex,QString,QStringList)) );

    applyThreading(mapping);

    model->cache()->setMessageThreading(mailbox.data(RoleMailboxName).toString(), mapping);
}

void ThreadingMsgListModel::applyThreading(const QVector<Imap::Responses::ThreadingNode> &mapping)
{
    if ( ! unknownUids.isEmpty() ) {
        // Some messages have UID zero, which means that they weren't loaded yet. Too bad.
        // FIXME: maybe we could re-use the response...
        qDebug() << unknownUids.size() << "messages have 0 UID";
        return;
    }

    emit layoutAboutToBeChanged();

    updatePersistentIndexesPhase1();

    _threading.clear();
    ptrToInternal.clear();
    // Default-construct the root node
    _threading[ 0 ].ptr = static_cast<MsgListModel*>( sourceModel() )->msgList;

    // At first, initialize threading nodes for all messages which are right now available in the mailbox.
    // We risk that we will have to delete some of them later on, but this is likely better than doing a lookup
    // for each UID individually (remember, the THREAD response might contain UIDs in crazy order).
    int upstreamMessages = sourceModel()->rowCount();
    QHash<uint,void*> uidToPtrCache;
    QSet<uint> usedNodes;
    for ( int i = 0; i < upstreamMessages; ++i ) {
        QModelIndex index = sourceModel()->index( i, 0 );
        ThreadNodeInfo node;
        node.uid = index.data( RoleMessageUid ).toUInt();
        if ( ! node.uid ) {
            throw UnknownMessageIndex("Encountered a message with zero UID when threading. This is a bug in Trojita, sorry.");
        }

        node.internalId = i + 1;
        node.ptr = static_cast<TreeItem*>( index.internalPointer() );
        uidToPtrCache[node.uid] = node.ptr;
        _threadingHelperLastId = node.internalId;
        // We're creating a new node here
        Q_ASSERT(!_threading.contains( node.internalId ));
        _threading[ node.internalId ] = node;
        ptrToInternal[ node.ptr ] = node.internalId;
    }

    // Mark the root node as always present
    usedNodes.insert(0);

    // Set up parents and find the list of all used nodes
    registerThreading( mapping, 0, uidToPtrCache, usedNodes );

    // Now remove all messages which were not referenced in the THREAD response from our mapping
    QHash<uint,ThreadNodeInfo>::iterator it = _threading.begin();
    while ( it != _threading.end() ) {
        if ( usedNodes.contains(it.key()) ) {
            // this message should be shown
            ++it;
        } else {
            // this message is not included in the list of messages actually to be shown
            ptrToInternal.remove(it->ptr);
            it = _threading.erase(it);
        }
    }

    updatePersistentIndexesPhase2();
    pruneTree();
    emit layoutChanged();
}

void ThreadingMsgListModel::registerThreading( const QVector<Imap::Responses::ThreadingNode> &mapping, uint parentId, const QHash<uint,void*> &uidToPtr, QSet<uint> &usedNodes )
{
    Q_FOREACH( const Imap::Responses::ThreadingNode &node, mapping ) {
        uint nodeId;
        if ( node.num == 0 ) {
            ThreadNodeInfo fake;
            fake.internalId = ++_threadingHelperLastId;
            fake.parent = parentId;
            Q_ASSERT(_threading.contains( parentId ));
            // The child will be registered to the list of parent's children after the if/else branch
            _threading[ fake.internalId ] = fake;
            nodeId = fake.internalId;
        } else {
            QHash<uint,void*>::const_iterator ptrIt = uidToPtr.find( node.num );
            if ( ptrIt == uidToPtr.constEnd() ) {
                QByteArray buf;
                QTextStream ss(&buf);
                ss << "The THREAD response references a message with UID " << node.num << ", which is not recognized at this point. ";
                ss << "More information is available in the IMAP protocol log.";
                ss.flush();
                throw UnknownMessageIndex(buf.constData());
            }

            QHash<void*,uint>::const_iterator nodeIt = ptrToInternal.constFind( *ptrIt );
            // The following assert would fail if there was a node with a valid UID, but not in our ptrToInternal mapping.
            // That is however non-issue, as we pre-create nodes for all messages beforehand.
            Q_ASSERT(nodeIt != ptrToInternal.constEnd());
            nodeId = *nodeIt;
        }
        _threading[ parentId ].children.append( nodeId );
        _threading[ nodeId ].parent = parentId;
        usedNodes.insert(nodeId);
        registerThreading( node.children, nodeId, uidToPtr, usedNodes );
    }
}

void ThreadingMsgListModel::updatePersistentIndexesPhase1()
{
    oldPersistentIndexes = persistentIndexList();
    oldPtrs.clear();
    Q_FOREACH( const QModelIndex &idx, oldPersistentIndexes ) {
        // the index could get invalidated by the pruneTree() or something else manipulating our _threading
        bool isOk = idx.isValid() && _threading.contains(idx.internalId());
        if (!isOk) {
            oldPtrs << 0;
            continue;
        }
        QModelIndex translated = mapToSource(idx);
        Q_ASSERT(translated.isValid());
        oldPtrs << translated.internalPointer();
    }
}

void ThreadingMsgListModel::updatePersistentIndexesPhase2()
{
    Q_ASSERT(oldPersistentIndexes.size() == oldPtrs.size());
    QList<QModelIndex> updatedIndexes;
    for ( int i = 0; i < oldPersistentIndexes.size(); ++i ) {
        QHash<void*,uint>::const_iterator ptrIt = ptrToInternal.constFind(oldPtrs[i]);
        if ( ptrIt == ptrToInternal.constEnd() ) {
            updatedIndexes.append( QModelIndex() );
            continue;
        }
        QHash<uint,ThreadNodeInfo>::const_iterator it = _threading.constFind(*ptrIt);
        Q_ASSERT(it != _threading.constEnd());
        QHash<uint,ThreadNodeInfo>::const_iterator parentNode = _threading.constFind( it->parent );
        Q_ASSERT(parentNode != _threading.constEnd());
        int offset = parentNode->children.indexOf(it->internalId);
        Q_ASSERT(offset != -1);
        updatedIndexes.append( createIndex( offset, oldPersistentIndexes[i].column(), it->internalId ) );
    }
    changePersistentIndexList( oldPersistentIndexes, updatedIndexes );
    oldPersistentIndexes.clear();
    oldPtrs.clear();
}

void ThreadingMsgListModel::pruneTree()
{
    // Our mapping (_threading) is completely unsorted, which means that we simply don't have any way of walking the tree from
    // the top. Instead, we got to work with a random walk, processing nodes in an unspecified order.  If we iterated on the QMap
    // directly, we'd hit an issue with iterator ordering (basically, we want to be able to say "hey, I don't care at which point
    // of the iteration I'm right now, the next node to process should be this one, and then we should resume with the rest").
    QList<uint> pending = _threading.keys();
    for (QList<uint>::iterator id = pending.begin(); id != pending.end(); /* nothing */) {
        // Convert to the hashmap
        QHash<uint, ThreadNodeInfo>::iterator it = _threading.find(*id);
        if (it == _threading.end()) {
            // We've already seen this node, that's due to promoting
            ++id;
            continue;
        }

        if (it->internalId == 0) {
            // A special root item; we should not delete that one :)
            ++id;
            continue;
        }
        if (it->ptr) {
            // regular and valid message -> skip
            ++id;
        } else {
            // a fake one

            // each node has a parent
            QHash<uint, ThreadNodeInfo>::iterator parent = _threading.find(it->parent);
            Q_ASSERT(parent != _threading.end());

            // and the node itself has to be found in its parent's children
            QList<uint>::iterator childIt = qFind(parent->children.begin(), parent->children.end(), it->internalId);
            Q_ASSERT(childIt != parent->children.end());

            if (it->children.isEmpty()) {
                // this is a leaf node, so we can just remove it
                parent->children.erase(childIt);
                _threading.erase(it);
                ++id;
            } else {
                // This node has some children, so we can't just delete it. Instead of that, we promote its first child
                // to replace this node.
                QHash<uint, ThreadNodeInfo>::iterator replaceWith = _threading.find(it->children.first());
                Q_ASSERT(replaceWith != _threading.end());

                *childIt = it->children.first();
                replaceWith->parent = parent->internalId;

                // set parent of all siblings of the just promoted node to the promoted node, and list them as children
                it->children.removeFirst();
                Q_FOREACH(const uint childId, it->children) {
                    QHash<uint, ThreadNodeInfo>::iterator sibling = _threading.find(childId);
                    Q_ASSERT(sibling != _threading.end());
                    sibling->parent = replaceWith.key();
                    replaceWith->children.append(sibling.key());
                }

                _threading.erase(it);

                if (!replaceWith->ptr) {
                    // If the just-promoted item is also a fake one, we'll have to visit it as well. This assignment is safe,
                    // because we've already processed the current item and are completely done with it. The worst which can
                    // happen is that we'll visit the same node twice, which is reasonably acceptable.
                    *id = replaceWith.key();
                }
            }
        }
    }
}

QDebug operator<<(QDebug debug, const ThreadNodeInfo &node)
{
    debug << "ThreadNodeInfo(" << node.internalId << node.uid << node.ptr<< node.parent << node.children << ")";
    return debug;
}

QStringList ThreadingMsgListModel::supportedCapabilities()
{
    return QStringList() << QLatin1String("THREAD=REFS") << QLatin1String("THREAD=REFERENCES") << QLatin1String("THREAD=ORDEREDSUBJECT");
}

QStringList ThreadingMsgListModel::mimeTypes() const
{
    return sourceModel() ? sourceModel()->mimeTypes() : QStringList();
}

QMimeData* ThreadingMsgListModel::mimeData( const QModelIndexList& indexes ) const
{
    if ( ! sourceModel() )
        return 0;

    QModelIndexList translated;
    Q_FOREACH(const QModelIndex &idx, indexes) {
        translated << mapToSource(idx);
    }
    return sourceModel()->mimeData(translated);
}

Qt::DropActions ThreadingMsgListModel::supportedDropActions() const
{
    return sourceModel() ? sourceModel()->supportedDropActions() : Qt::DropActions(0);
}

bool ThreadingMsgListModel::threadContainsUnreadMessages(const uint root) const
{
    // FIXME: cache the value somewhere...
    QList<uint> queue;
    queue.append(root);
    while ( ! queue.isEmpty() ) {
        uint current = queue.takeFirst();
        QHash<uint,ThreadNodeInfo>::const_iterator it = _threading.constFind(current);
        Q_ASSERT(it != _threading.constEnd());
        Q_ASSERT(it->ptr);
        TreeItemMessage *message = dynamic_cast<TreeItemMessage*>(it->ptr);
        Q_ASSERT(message);
        if ( ! message->isMarkedAsRead() )
            return true;
        queue.append(it->children);
    }
    return false;
}

}
}
