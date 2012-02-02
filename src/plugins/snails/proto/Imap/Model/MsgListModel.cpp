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

#include "MsgListModel.h"
#include "MailboxTree.h"
#include "MailboxModel.h"

#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QIcon>
#include <QMimeData>

namespace Imap {
namespace Mailbox {

MsgListModel::MsgListModel( QObject* parent, Model* model ): QAbstractProxyModel(parent), msgList(0), waitingForMessages(false)
{
    setSourceModel( model );

    // FIXME: will need to be expanded when Model supports more signals...
    connect( model, SIGNAL( modelAboutToBeReset() ), this, SLOT( resetMe() ) );
    connect( model, SIGNAL( layoutAboutToBeChanged() ), this, SIGNAL( layoutAboutToBeChanged() ) );
    connect( model, SIGNAL( layoutChanged() ), this, SIGNAL( layoutChanged() ) );
    connect( model, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
            this, SLOT( handleDataChanged( const QModelIndex&, const QModelIndex& ) ) );
    connect( model, SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsAboutToBeRemoved(const QModelIndex&, int,int ) ) );
    connect( model, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsRemoved(const QModelIndex&, int,int ) ) );
    connect( model, SIGNAL( rowsAboutToBeInserted( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsAboutToBeInserted(const QModelIndex&, int,int ) ) );
    connect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
             this, SLOT( handleRowsInserted(const QModelIndex&, int,int ) ) );
}

void MsgListModel::handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    QModelIndex first = mapFromSource( topLeft );
    QModelIndex second = mapFromSource( bottomRight );

    if ( ! first.isValid() || ! second.isValid() ) {
        return;
    }

    second = createIndex( second.row(), COLUMN_COUNT - 1, Model::realTreeItem( second ) );

    if ( first.parent() == second.parent() ) {
        emit dataChanged( first, second );
    } else {
        Q_ASSERT(false);
        return;
    }
}

QModelIndex MsgListModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( ! msgList )
        return QModelIndex();

    if ( parent.isValid() )
        return QModelIndex();

    if ( column < 0 || column >= COLUMN_COUNT )
        return QModelIndex();

    Model* model = dynamic_cast<Model*>( sourceModel() );
    Q_ASSERT( model );

    if ( row >= static_cast<int>( msgList->rowCount( model ) ) || row < 0 )
        return QModelIndex();

    return createIndex( row, column, msgList->child( row, model ) );
}

QModelIndex MsgListModel::parent( const QModelIndex& index ) const
{
    Q_UNUSED( index );
    return QModelIndex();
}

bool MsgListModel::hasChildren( const QModelIndex& parent ) const
{
    return ! parent.isValid();
}

int MsgListModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() )
        return 0;

    if ( ! msgList )
        return 0;

    return msgList->rowCount( dynamic_cast<Model*>( sourceModel() ) );
}

int MsgListModel::columnCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() )
        return 0;
    if ( parent.column() != 0 && parent.column() != -1 )
        return 0;
    return COLUMN_COUNT;
}

QModelIndex MsgListModel::mapToSource( const QModelIndex& proxyIndex ) const
{
    if ( ! msgList )
        return QModelIndex();

    if ( proxyIndex.parent().isValid() )
        return QModelIndex();

    Model* model = dynamic_cast<Model*>( sourceModel() );
    Q_ASSERT( model );

    return model->createIndex( proxyIndex.row(), 0, msgList->child( proxyIndex.row(), model ) );
}

QModelIndex MsgListModel::mapFromSource( const QModelIndex& sourceIndex ) const
{
    if ( ! msgList )
        return QModelIndex();

    if ( sourceIndex.model() != sourceModel() )
        return QModelIndex();
    if ( dynamic_cast<TreeItemMessage*>( Model::realTreeItem( sourceIndex ) ) ) {
        return index( sourceIndex.row(), 0, QModelIndex() );
    } else {
        return QModelIndex();
    }
}

QVariant MsgListModel::data( const QModelIndex& proxyIndex, int role ) const
{
    if ( ! msgList )
        return QVariant();

    if ( ! proxyIndex.isValid() || proxyIndex.model() != this )
        return QVariant();

    TreeItemMessage* message = dynamic_cast<TreeItemMessage*>( Model::realTreeItem( proxyIndex ) );
    Q_ASSERT( message );

    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            switch ( proxyIndex.column() ) {
                case SUBJECT:
                    return QAbstractProxyModel::data( proxyIndex, Qt::DisplayRole );
                case FROM:
                    return QString::fromAscii("[from]");
                case TO:
                    return QString::fromAscii("[to]");
                case CC:
                    return QString::fromAscii("[cc]");
                case BCC:
                    return QString::fromAscii("[bcc]");
                case DATE:
                    return message->envelope( static_cast<Model*>( sourceModel() ) ).date;
                case SIZE:
                    return message->size( static_cast<Model*>( sourceModel() ) );
                default:
                    return QVariant();
            }

        case RoleIsFetched:
        case RoleMessageUid:
        case RoleMessageIsMarkedDeleted:
        case RoleMessageIsMarkedRead:
        case RoleMessageIsMarkedForwarded:
        case RoleMessageIsMarkedReplied:
        case RoleMessageIsMarkedRecent:
        case RoleMessageDate:
        case RoleMessageFrom:
        case RoleMessageTo:
        case RoleMessageCc:
        case RoleMessageBcc:
        case RoleMessageSender:
        case RoleMessageReplyTo:
        case RoleMessageInReplyTo:
        case RoleMessageMessageId:
        case RoleMessageSubject:
        case RoleMessageFlags:
        case RoleMessageSize:
            return dynamic_cast<TreeItemMessage*>( Model::realTreeItem(
                    proxyIndex ) )->data( static_cast<Model*>( sourceModel() ), role );
        default:
            return QAbstractProxyModel::data( createIndex( proxyIndex.row(), 0, proxyIndex.internalPointer() ), role );
    }
}

QVariant MsgListModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QAbstractItemModel::headerData( section, orientation, role );

    switch ( section ) {
        case SUBJECT:
            return tr( "Subject" );
        case FROM:
            return tr( "From" );
        case TO:
            return tr( "To" );
        case CC:
            return tr("Cc");
        case BCC:
            return tr("Bcc");
        case DATE:
            return tr( "Date" );
        case SIZE:
            return tr( "Size" );
        default:
            return QVariant();
    }
}

Qt::ItemFlags MsgListModel::flags( const QModelIndex& index ) const
{
    if ( ! index.isValid() || index.model() != this )
        return QAbstractProxyModel::flags( index );

    TreeItemMessage* message = dynamic_cast<TreeItemMessage*>(
            Model::realTreeItem( index ) );
    Q_ASSERT( message );

    if ( ! message->fetched() )
        return QAbstractProxyModel::flags( index );

    return Qt::ItemIsDragEnabled | QAbstractProxyModel::flags( index );
}

Qt::DropActions MsgListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList MsgListModel::mimeTypes() const
{
    return QStringList() << QLatin1String("application/x-trojita-message-list");
}

QMimeData* MsgListModel::mimeData( const QModelIndexList& indexes ) const
{
    if ( indexes.isEmpty() )
        return 0;

    QMimeData* res = new QMimeData();
    QByteArray encodedData;
    QDataStream stream( &encodedData, QIODevice::WriteOnly );
    stream.setVersion(QDataStream::Qt_4_6);

    TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( Model::realTreeItem(
            indexes.front() )->parent()->parent() );
    Q_ASSERT( mailbox );
    stream << mailbox->mailbox() << mailbox->syncState.uidValidity();

    QList<uint> uids;
    for ( QModelIndexList::const_iterator it = indexes.begin(); it != indexes.end(); ++it ) {
        TreeItemMessage* message = dynamic_cast<TreeItemMessage*>( Model::realTreeItem( *it ) );
        Q_ASSERT( message );
        Q_ASSERT( message->fetched() ); // should've been handled by flags()
        Q_ASSERT( message->parent()->parent() == mailbox );
        Q_ASSERT( message->uid() > 0 );
        uids << message->uid();
    }
    stream << uids;
    res->setData( QLatin1String("application/x-trojita-message-list"), encodedData );
    return res;
}

void MsgListModel::resetMe()
{
    setMailbox( QModelIndex() );
}

void MsgListModel::handleRowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    if ( ! parent.isValid() ) {
        // Top-level items are tricky :(. As a quick hack, let's just die.
        resetMe();
        return;
    }

    if ( ! msgList )
        return;

    TreeItem* item = Model::realTreeItem( parent );
    TreeItemMailbox* mailbox = dynamic_cast<TreeItemMailbox*>( item );
    TreeItemMsgList* newList = dynamic_cast<TreeItemMsgList*>( item );

    if ( parent.isValid() ) {
        Q_ASSERT( parent.model() == sourceModel() );
    } else {
        // a top-level mailbox might have been deleted, so we gotta setup proper pointer
        mailbox = static_cast<Model*>( sourceModel() )->_mailboxes;
        Q_ASSERT( mailbox );
    }

    if ( newList ) {
        if ( newList == msgList ) {
            beginRemoveRows( mapFromSource( parent ), start, end );
            for ( int i = start; i <= end; ++i )
                emit messageRemoved( msgList->child( i, static_cast<Model*>( sourceModel() ) ) );
        }
    } else if ( mailbox ) {
        Q_ASSERT( start > 0 );
        // if we're below it, we're gonna die
        for ( int i = start; i <= end; ++i ) {
            const Model* model = 0;
            QModelIndex translatedParent;
            Model::realTreeItem( parent, &model, &translatedParent );
            // FIXME: this assumes that no rows were removed by the proxy model
            TreeItemMailbox* m = dynamic_cast<TreeItemMailbox*>( static_cast<TreeItem*>( model->index( i, 0, translatedParent ).internalPointer() ) );
            Q_ASSERT( m );
            TreeItem* up = msgList->parent();
            while ( up ) {
                if ( m == up ) {
                    resetMe();
                    return;
                }
                up = up->parent();
            }
        }
    }
}

void MsgListModel::handleRowsRemoved( const QModelIndex& parent, int start, int end )
{
    Q_UNUSED( start );
    Q_UNUSED( end );

    if ( ! msgList )
        return;

    if ( ! parent.isValid() ) {
        // already handled by resetMe() in handleRowsAboutToBeRemoved()
        return;
    }

    if( dynamic_cast<TreeItemMsgList*>( Model::realTreeItem( parent ) ) == msgList )
        endRemoveRows();
}

void MsgListModel::handleRowsAboutToBeInserted( const QModelIndex& parent, int start, int end )
{
    if ( ! parent.isValid() )
        return;

    TreeItemMsgList* newList = dynamic_cast<TreeItemMsgList*>( Model::realTreeItem( parent ) );
    if ( msgList && msgList == newList ) {
        beginInsertRows( mapFromSource( parent), start, end );
    }
}

void MsgListModel::handleRowsInserted( const QModelIndex& parent, int start, int end )
{
    if ( ! parent.isValid() )
        return;

    Q_UNUSED( start );
    Q_UNUSED( end );
    TreeItemMsgList* newList = dynamic_cast<TreeItemMsgList*>( Model::realTreeItem( parent ) );
    if ( msgList && msgList == newList ) {
        endInsertRows();
    }

    if ( waitingForMessages ) {
        waitingForMessages = false;
        emit messagesAvailable();
    }
}

void MsgListModel::setMailbox( const QModelIndex& index )
{
    waitingForMessages = true;
    if ( ! index.isValid() ) {
        msgList = 0;
        reset();
        emit mailboxChanged();
        return;
    }

    const Model* model = 0;
    TreeItemMailbox* mbox = dynamic_cast<TreeItemMailbox*>( Model::realTreeItem( index, &model ));
    Q_ASSERT( mbox );
    if ( ! mbox->isSelectable() )
        return;
    TreeItemMsgList* newList = dynamic_cast<TreeItemMsgList*>(
            mbox->child( 0, const_cast<Model*>( model ) ) );
    Q_ASSERT( newList );
    if ( newList != msgList && mbox->isSelectable() ) {
        msgList = newList;
        reset();
        emit mailboxChanged();
        // We want to tell the Model that it should consider starting the IDLE command.
        const_cast<Model*>( model )->switchToMailbox( index );
    }
}

TreeItemMailbox* MsgListModel::currentMailbox() const
{
    return msgList ? dynamic_cast<TreeItemMailbox*>( msgList->parent() ) : 0;
}

}
}
