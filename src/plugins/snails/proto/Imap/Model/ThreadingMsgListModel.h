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

#ifndef IMAP_THREADINGMSGLISTMODEL_H
#define IMAP_THREADINGMSGLISTMODEL_H

#include <QAbstractProxyModel>
#include "Imap/Parser/Response.h"

class QTimer;

/** @short Namespace for IMAP interaction */
namespace Imap {

/** @short Classes for handling of mailboxes and connections */
namespace Mailbox {

class TreeItem;

/** @short A node in tree structure used for threading representation */
struct ThreadNodeInfo {
    /** @short Internal unique identifier used for model indexes */
    uint internalId;
    /** @short A UID of the message in a mailbox */
    uint uid;
    /** @short internalId of a parent of this message */
    uint parent;
    /** @short List of children of current node */
    QList<uint> children;
    /** @short Pointer to the TreeItemMessage* of the corresponding message */
    TreeItem *ptr;
    ThreadNodeInfo(): internalId(0), uid(0), parent(0), ptr(0) {}
};

QDebug operator<<(QDebug debug, const ThreadNodeInfo &node);

/** @short A model implementing view of the whole IMAP server

The problem with threading is that due to the extremely asynchronous nature of the IMAP Model, we often get informed about indexes
to messages which "just arrived", and therefore do not have even their UID available. That sucks, because we have to somehow handle
them.  Situation gets a bit more complicated by the initial syncing -- this ThreadingMsgListModel can't tell whether the rowsInserted()
signals mean that the underlying model is getting populated, or whether it's a sign of a just-arrived message.  On a plus side, the Model
guarantees that the only occurence when a message could have UID 0 is when the mailbox has been synced previously, and the message is a new
arrival.  In all other contexts (that is, during the mailbox re-synchronization), there is a hard guarantee that the UID of any message
available via the MVC API will always be non-zero.

The model should also refrain from sending extra THREAD commands to the server, and cache the responses locally.  This is pretty easy for
message deletions, as it should be only a matter of replacing some node in the threading info with a fake ThreadNodeInfo node and running
the pruneTree() method, except that we might not know the UID of the message in question, and hence can't know what to delete.

*/
class ThreadingMsgListModel: public QAbstractProxyModel {
    Q_OBJECT

public:
    ThreadingMsgListModel( QObject *parent );
    virtual void setSourceModel( QAbstractItemModel *sourceModel );

    virtual QModelIndex index( int row, int column, const QModelIndex& parent=QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex& index ) const;
    virtual int rowCount( const QModelIndex& parent=QModelIndex() ) const;
    virtual int columnCount( const QModelIndex& parent=QModelIndex() ) const;
    virtual QModelIndex mapToSource( const QModelIndex& proxyIndex ) const;
    virtual QModelIndex mapFromSource( const QModelIndex& sourceIndex ) const;
    virtual bool hasChildren( const QModelIndex& parent=QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &proxyIndex, int role ) const;
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual QStringList mimeTypes() const;
    virtual QMimeData* mimeData( const QModelIndexList& indexes ) const;
    virtual Qt::DropActions supportedDropActions() const;

    /** @short List of capabilities which could be used for threading

    If any of them are present in server's capabilities, at least some level of threading will be possible.
*/
    static QStringList supportedCapabilities();

public slots:
    void resetMe();
    void handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );
    void handleRowsAboutToBeRemoved( const QModelIndex& parent, int start, int end );
    void handleRowsRemoved( const QModelIndex& parent, int start, int end );
    void handleRowsAboutToBeInserted( const QModelIndex& parent, int start, int end );
    void handleRowsInserted( const QModelIndex& parent, int start, int end );
    /** @short Feed this with the data from a THREAD response */
    void slotThreadingAvailable( const QModelIndex &mailbox, const QString &algorithm,
                                 const QStringList &searchCriteria,
                                 const QVector<Imap::Responses::ThreadingNode> &mapping );
    void slotThreadingFailed(const QModelIndex &mailbox, const QString &algorithm,
                             const QStringList &searchCriteria);
    /** @short Really apply threading to this model */
    void applyThreading(const QVector<Imap::Responses::ThreadingNode> &mapping);

private slots:
    /** @short Display messages without any threading at all, as a liner list */
    void updateNoThreading();
    /** @short Ask the model for a THREAD response */
    void askForThreading();

private:
    /** @short Update QAbstractItemModel's idea of persistent indexes after a threading change */
    void updatePersistentIndexesPhase1();
    void updatePersistentIndexesPhase2();
    /** @short Convert the threading from a THREAD response and apply that threading to this model */
    void registerThreading( const QVector<Imap::Responses::ThreadingNode> &mapping, uint parentId,
                            const QHash<uint,void*> &uidToPtr, QSet<uint> &usedNodes );

    /** @short Remove fake messages from the threading tree

    @return True if any fake messages got removed, false otherwise
    */
    void pruneTree();

    /** @short Check current thread for "unread messages" */
    bool threadContainsUnreadMessages(const uint root) const;

    /** @short Is this someone else's THREAD response? */
    bool shouldIgnoreThisThreadingResponse(const QModelIndex &mailbox, const QString &algorithm,
                                           const QStringList &searchCriteria, const Model **realModel=0);

    ThreadingMsgListModel& operator=( const ThreadingMsgListModel& ); // don't implement
    ThreadingMsgListModel( const ThreadingMsgListModel& ); // don't implement

    /** @short Mapping from the upstream model's internalId to ThreadingMsgListModel's internal IDs */
    QHash<void*,uint> ptrToInternal;
    /** @short Tree for the threading

    This tree is indexed by our internal ID.
    */
    QHash<uint,ThreadNodeInfo> _threading;
    /** @short Last assigned internal ID */
    uint _threadingHelperLastId;
    /** @short Messages with unkown UIDs */
    QList<QPersistentModelIndex> unknownUids;

    /** @short Threading algorithm we're using for this request */
    QString requestedAlgorithm;

    /** @short Recursion guard for "is the model currently being reset?"

    We can't be sure what happens when we call rowCount() from updateNoThreading(). It is
    possible that the rowCount() would propagate to Model's _askForMessagesInMailbox(),
    which could in turn call beginInsertRows, leading to a possible recursion.
 */
    bool modelResetInProgress;

    QModelIndexList oldPersistentIndexes;
    QList<void*> oldPtrs;
};

}

}

#endif /* IMAP_THREADINGMSGLISTMODEL_H */
