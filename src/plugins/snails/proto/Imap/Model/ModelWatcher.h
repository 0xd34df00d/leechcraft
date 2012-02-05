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
#ifndef MODELWATCHER_H
#define MODELWATCHER_H

#include <QModelIndex>
#include <QAbstractItemModel>

namespace Imap {

namespace Mailbox {

class ModelWatcher : public QObject
{
Q_OBJECT
public:
    ModelWatcher( QObject* parent=0 ): QObject(parent) {};
    void setModel( QAbstractItemModel* model );

private slots:
    void columnsAboutToBeInserted( const QModelIndex&, int, int );
    void columnsAboutToBeRemoved( const QModelIndex&, int, int );
    void columnsInserted( const QModelIndex&, int, int );
    void columnsRemoved( const QModelIndex&, int, int );
    void dataChanged( const QModelIndex&, const QModelIndex& );
    void headerDataChanged( Qt::Orientation, int, int );
    void layoutAboutToBeChanged();
    void layoutChanged();
    void modelAboutToBeReset();
    void modelReset();
    void rowsAboutToBeInserted( const QModelIndex&, int, int );
    void rowsAboutToBeRemoved( const QModelIndex&, int, int );
    void rowsInserted( const QModelIndex&, int, int );
    void rowsRemoved( const QModelIndex&, int, int );

private:
    QAbstractItemModel* _model;
};

}

}

#endif // MODELWATCHER_H
