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
#include "PrettyMsgListModel.h"
#include "MsgListModel.h"
#include "ItemRoles.h"
#include "Utils.h"

#include <QFont>
#include "Gui/IconLoader.h"

namespace Imap {

namespace Mailbox {

PrettyMsgListModel::PrettyMsgListModel( QObject *parent ): QSortFilterProxyModel( parent )
{
    setDynamicSortFilter( true );
}

QVariant PrettyMsgListModel::data( const QModelIndex& index, int role ) const
{
    if ( ! index.isValid() || index.model() != this )
        return QVariant();

    if ( index.column() < 0 || index.column() >= columnCount( index.parent() ) )
        return QVariant();

    if ( index.row() < 0 || index.row() >= rowCount( index.parent() ) )
        return QVariant();

    QModelIndex translated = mapToSource( index );

    switch ( role ) {

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        switch ( index.column() ) {
        case MsgListModel::TO:
        case MsgListModel::FROM:
        case MsgListModel::CC:
        case MsgListModel::BCC:
            {
                int backendRole = 0;
                switch ( index.column() ) {
                case MsgListModel::FROM:
                    backendRole = RoleMessageFrom;
                    break;
                case MsgListModel::TO:
                    backendRole = RoleMessageTo;
                    break;
                case MsgListModel::CC:
                    backendRole = RoleMessageCc;
                    break;
                case MsgListModel::BCC:
                    backendRole = RoleMessageBcc;
                    break;
                }
                QVariantList items = translated.data( backendRole ).toList();
                return Imap::Message::MailAddress::prettyList( items, role == Qt::DisplayRole ?
                                                               Imap::Message::MailAddress::FORMAT_JUST_NAME :
                                                               Imap::Message::MailAddress::FORMAT_READABLE );
            }
        case MsgListModel::DATE:
            {
                QDateTime res = translated.data( RoleMessageDate ).toDateTime();
                if ( res.date() == QDate::currentDate() )
                    return res.time().toString( Qt::SystemLocaleShortDate );
                else
                    return res.toString( Qt::SystemLocaleShortDate );
            }
        case MsgListModel::SIZE:
            {
                QVariant size = translated.data(RoleMessageSize);
                if (!size.isValid()) {
                    return QVariant();
                }
                return PrettySize::prettySize(size.toUInt());
            }
        }
        break;


    case Qt::TextAlignmentRole:
        switch ( index.column() ) {
        case MsgListModel::SIZE:
            return Qt::AlignRight;
        default:
            return QVariant();
        }

    case Qt::DecorationRole:
        switch ( index.column() ) {
        case MsgListModel::SUBJECT:
            {
            if ( ! translated.data( RoleIsFetched ).toBool() )
                return QVariant();

            bool isForwarded = translated.data( RoleMessageIsMarkedForwarded ).toBool();
            bool isReplied = translated.data( RoleMessageIsMarkedReplied ).toBool();

            if ( translated.data( RoleMessageIsMarkedDeleted ).toBool() )
                return Gui::loadIcon(QLatin1String("mail-deleted"));
            else if ( isForwarded && isReplied )
                return Gui::loadIcon(QLatin1String("mail-replied-forw"));
            else if ( isReplied )
                return Gui::loadIcon(QLatin1String("mail-replied"));
            else if ( isForwarded )
                return Gui::loadIcon(QLatin1String("mail-forwarded"));
            else if ( translated.data( RoleMessageIsMarkedRecent ).toBool() )
                return Gui::loadIcon(QLatin1String("mail-recent"));
            else
                return QIcon(QLatin1String(":/icons/transparent.png"));
            }
        case MsgListModel::SEEN:
            if ( ! translated.data( RoleIsFetched ).toBool() )
                return QVariant();
            if ( ! translated.data( RoleMessageIsMarkedRead ).toBool() )
                return QIcon(QLatin1String(":/icons/mail-unread.png"));
            else
                return QIcon(QLatin1String(":/icons/mail-read.png"));
        default:
            return QVariant();
        }

    case Qt::FontRole:
        {
            if ( ! translated.data( RoleIsFetched ).toBool() )
                return QVariant();

            QFont font;
            if ( translated.data( RoleMessageIsMarkedDeleted ).toBool() )
                font.setStrikeOut( true );

            if ( ! translated.data( RoleMessageIsMarkedRead ).toBool() ) {
                // If any message is marked as unread, show it in bold and be done with it
                font.setBold( true );
            } else if ( translated.model()->hasChildren(translated) && translated.data(RoleThreadRootWithUnreadMessages).toBool() ) {
                // If this node is not marked as read, is a root of some thread and that thread
                // contains unread messages, display the thread's root underlined
                font.setUnderline(true);
            }

            return font;
        }
    }

    return QSortFilterProxyModel::data( index, role );
}

}

}
