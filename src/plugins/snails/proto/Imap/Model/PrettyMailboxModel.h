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
#ifndef PRETTYMAILBOXMODEL_H
#define PRETTYMAILBOXMODEL_H

#include <QSortFilterProxyModel>
#include "Imap/Model/MailboxModel.h"

namespace Imap {

namespace Mailbox {

/** @short A pretty proxy model which increases sexiness of the MailboxModel */
class PrettyMailboxModel: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    PrettyMailboxModel( QObject* parent, MailboxModel* mailboxModel );
    virtual QVariant data( const QModelIndex& index, int role ) const;
    virtual bool filterAcceptsColumn( int source_column, const QModelIndex& source_parent ) const;
    /** @short Override in order to prevent needless LIST commands */
    virtual bool hasChildren(const QModelIndex &parent=QModelIndex()) const;
#ifdef XTUPLE_CONNECT
    void xtConnectStatusChanged(QModelIndex index);
#endif
};

}

}

#endif // PRETTYMAILBOXMODEL_H
