/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filestreemodel.h"
#include <QIODevice>
#include <QMimeData>
#include <QtDebug>
#include <QTreeView>
#include <QMenu>
#include "interfaces/netstoremanager/isupportfilelistings.h"

namespace LC
{
namespace NetStoreManager
{
	Qt::DropActions FilesTreeModel::supportedDropActions () const
	{
		return Qt::MoveAction | Qt::CopyAction;
	}

	QStringList FilesTreeModel::mimeTypes () const
	{
		return { "x-leechcraft/nsm-item" };
	}

	QMimeData* FilesTreeModel::mimeData (const QModelIndexList& indexes) const
	{
		QMimeData *mimeData = new QMimeData ();
		QByteArray encodedData;

		QDataStream stream (&encodedData, QIODevice::WriteOnly);

		for (const auto& index : indexes)
			if (index.isValid () &&
					index.column () == 0)
				stream << data (index).toString ()
						<< data (index, ListingRole::ID).toByteArray ()
						<< data (index, ListingRole::InTrash).toBool ()
						<< data (index, ListingRole::IsDirectory).toBool ()
						<< index.parent ().data (ListingRole::ID).toByteArray ();

		mimeData->setData ("x-leechcraft/nsm-item", encodedData);
		return mimeData;
	}
}
}
