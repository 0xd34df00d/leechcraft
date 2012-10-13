/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "filesmodel.h"
#include <QMimeData>
#include <QtDebug>
#include <QTreeView>
#include <QMenu>
#include "interfaces/netstoremanager/isupportfilelistings.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	FilesModel::FilesModel (QObject *parent)
	: QStandardItemModel (parent)
	{
	}

	Qt::DropActions FilesModel::supportedDropActions () const
	{
		return Qt::MoveAction | Qt::CopyAction;
	}

	QStringList FilesModel::mimeTypes () const
	{
		return { "x-leechcraft/nsm-item" };
	}

	QMimeData* FilesModel::mimeData (const QModelIndexList& indexes) const
	{
		QMimeData *mimeData = new QMimeData ();
		QByteArray encodedData;

		QDataStream stream (&encodedData, QIODevice::WriteOnly);

		Q_FOREACH (const QModelIndex& index, indexes)
			if (index.isValid ())
				stream << data (index).toString ()
						<< data (index, ListingRole::ID).toStringList ()
						<< data (index, ListingRole::InTrash).toBool ()
						<< data (index, ListingRole::Directory).toBool ()
						<< index.parent ().data (ListingRole::ID).toStringList ();

		mimeData->setData ("x-leechcraft/nsm-item", encodedData);
		return mimeData;
	}
}
}