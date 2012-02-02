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
#include "PrettyMailboxModel.h"
#include "MailboxModel.h"
#include "ItemRoles.h"

#ifdef XTUPLE_CONNECT
# include "Common/SettingsNames.h"
# include <QSettings>
#endif

#include <QFont>
#include "Gui/IconLoader.h"

namespace Imap {

namespace Mailbox {

PrettyMailboxModel::PrettyMailboxModel( QObject* parent, MailboxModel* mailboxModel ):
        QSortFilterProxyModel( parent )
{
    setDynamicSortFilter(true);
    setSourceModel(mailboxModel);
}

QVariant PrettyMailboxModel::data( const QModelIndex& index, int role ) const
{
    if ( ! index.isValid() )
        return QVariant();

    if ( index.column() != 0 )
        return QVariant();

    if ( index.row() < 0 || index.row() >= rowCount( index.parent() ) || index.model() != this )
        return QVariant();

    switch ( role ) {
        case Qt::DisplayRole:
            {
            QModelIndex translated = mapToSource( index );
            qlonglong unreadCount = translated.data( RoleUnreadMessageCount ).toLongLong();
            if ( unreadCount )
                return tr("%1 (%2)").arg(
                        QSortFilterProxyModel::data( index, RoleShortMailboxName ).toString() ).arg(
                        unreadCount );
            else
                return QSortFilterProxyModel::data( index, RoleShortMailboxName );
            }
        case Qt::FontRole:
            {
            QModelIndex translated = mapToSource( index );
            if ( translated.data( RoleMailboxNumbersFetched ).toBool() &&
                 translated.data( RoleUnreadMessageCount ).toULongLong() > 0 ) {
                QFont font;
                font.setBold( true );
                return font;
            } else {
                return QVariant();
            }
            }
        case Qt::DecorationRole:
            {
            QModelIndex translated = mapToSource( index );
            if ( translated.data( RoleMailboxItemsAreLoading ).toBool() )
                return Gui::loadIcon(QLatin1String("folder-grey"));
#ifdef XTUPLE_CONNECT
            else if ( QSettings().value( Common::SettingsNames::xtSyncMailboxList ).toStringList().contains(
                    translated.data( RoleMailboxName ).toString() ) )
                return Gui::loadIcon(QLatin1String("folder-xt-sync.png"));
#endif
            else if ( translated.data( RoleMailboxIsINBOX ).toBool() )
                return Gui::loadIcon(QLatin1String("mail-folder-inbox"));
            else if ( translated.data( RoleRecentMessageCount ).toInt() > 0 )
                return Gui::loadIcon(QLatin1String("folder-bookmark"));
            else if ( translated.data( RoleMailboxIsSelectable ).toBool() )
                return Gui::loadIcon(QLatin1String("folder"));
            else
                return Gui::loadIcon(QLatin1String("folder-open"));
            }
        default:
            return QSortFilterProxyModel::data( index, role );
    }
}

bool PrettyMailboxModel::filterAcceptsColumn( int source_column, const QModelIndex& source_parent ) const
{
    Q_UNUSED(source_parent);
    return source_column == 0;
}

bool PrettyMailboxModel::hasChildren(const QModelIndex& parent) const
{
    return dynamic_cast<const MailboxModel*>(sourceModel())->hasChildren(mapToSource(parent));
}

#ifdef XTUPLE_CONNECT
void PrettyMailboxModel::xtConnectStatusChanged(QModelIndex index)
{
    Q_ASSERT(index.model() == this);
    emit dataChanged(index, index);
}
#endif

}

}
