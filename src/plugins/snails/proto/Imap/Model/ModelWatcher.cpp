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
#include <QDebug>
#include "ModelWatcher.h"

namespace Imap {

namespace Mailbox {

void ModelWatcher::setModel( QAbstractItemModel* model )
{
    connect( model, SIGNAL( columnsAboutToBeInserted( const QModelIndex&, int, int ) ),
             this, SLOT( columnsAboutToBeInserted( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( columnsAboutToBeRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( columnsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( rowsAboutToBeInserted( const QModelIndex&, int, int ) ),
             this, SLOT( rowsAboutToBeInserted( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
             this, SLOT( columnsInserted( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( columnsRemoved( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
             this, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
             this, SLOT( rowsRemoved( const QModelIndex&, int, int ) ) );
    connect( model, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
             this, SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
    connect( model, SIGNAL( headerDataChanged( Qt::Orientation, int, int ) ),
             this, SLOT( headerDataChanged( Qt::Orientation, int, int ) ) );
    connect( model, SIGNAL( layoutAboutToBeChanged() ), this, SLOT( layoutAboutToBeChanged() ) );
    connect( model, SIGNAL( layoutChanged() ), this, SLOT( layoutChanged() ) );
    connect( model, SIGNAL( modelAboutToBeReset() ), this, SLOT( modelAboutToBeReset() ) );
    connect( model, SIGNAL( modelReset() ), this, SLOT( modelReset() ) );
    _model = model;
}

void ModelWatcher::columnsAboutToBeInserted( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "columnsAboutToBeInserted(" << parent << start << end << ")";
}

void ModelWatcher::columnsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "columnsAboutToBeRemoved(" << parent << start << end << ")";
}

void ModelWatcher::columnsInserted( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "columnsInserted(" << parent << start << end << ")";
}

void ModelWatcher::columnsRemoved( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "columnsRemoved(" << parent << start << end << ")";
}

void ModelWatcher::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    qDebug() << sender()->objectName() << "dataChanged(" << topLeft << bottomRight << ")";
    qDebug() << "new data" << _model->data( topLeft, Qt::DisplayRole );
}
void ModelWatcher::headerDataChanged( Qt::Orientation orientation, int first, int last )
{
    qDebug() << sender()->objectName() << "headerDataChanged(" << orientation << first << last << ")";
}
void ModelWatcher::layoutAboutToBeChanged()
{
    qDebug() << sender()->objectName() << "layoutAboutToBeChanged()";
}

void ModelWatcher::layoutChanged()
{
    qDebug() << sender()->objectName() << "layoutChanged()";
}

void ModelWatcher::modelAboutToBeReset()
{
    qDebug() << sender()->objectName() << "modelAboutToBeReset()";
}

void ModelWatcher::modelReset()
{
    qDebug() << sender()->objectName() << "modelReset()";
}

void ModelWatcher::rowsAboutToBeInserted( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "rowsAboutToBeInserted(" << parent << start << end << ")";
}

void ModelWatcher::rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "rowsAboutToBeRemoved(" << parent << start << end << ")";
}

void ModelWatcher::rowsInserted( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "rowsInserted(" << parent << start << end << ")";
}

void ModelWatcher::rowsRemoved( const QModelIndex& parent, int start, int end )
{
    qDebug() << sender()->objectName() << "rowsRemoved(" << parent << start << end << ")";
}

}

}
