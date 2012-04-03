/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "storagemodel.h"
#include "core.h"
#include "todostorage.h"
#include <interfaces/core/itagsmanager.h>

namespace LeechCraft
{
namespace Otlozhu
{
	StorageModel::StorageModel (QObject *parent)
	: QAbstractItemModel (parent)
	, Storage_ (0)
	{
		Headers_ << tr ("Title")
				<< tr ("Tags")
				<< tr ("Due date")
				<< tr ("Created")
				<< tr ("Percentage");
	}

	QVariant StorageModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
			return QVariant ();
		return Headers_ [section];
	}

	QModelIndex StorageModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (parent.isValid ())
			return QModelIndex ();
		return createIndex (row, column);
	}

	QModelIndex StorageModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int StorageModel::columnCount (const QModelIndex&) const
	{
		return Columns::MAX;
	}

	int StorageModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : (Storage_ ? Storage_->GetNumItems () : 0);
	}

	QVariant StorageModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		if (role != Qt::DisplayRole)
			return QVariant ();

		const auto item = Storage_->GetItemAt (index.row ());
		switch (index.column ())
		{
		case Columns::Title:
			return item->GetTitle ();
		case Columns::Tags:
		{
			const auto& ids = item->GetTagIDs ();
			if (ids.isEmpty ())
				return QString ();
			return Core::Instance ().GetProxy ()->GetTagsManager ()->JoinIDs (ids);
		}
		case Columns::DueDate:
		{
			const auto& date = item->GetDueDate ();
			return date.isNull () ? tr ("not set") : date.toString ();
		}
		case Columns::Created:
			return item->GetCreatedDate ();
		case Columns::Percentage:
			return item->GetPercentage ();
		default:
			return QVariant ();
		}
	}

	void StorageModel::SetStorage (TodoStorage *storage)
	{
		if (Storage_)
			disconnect (Storage_,
					0,
					this,
					0);
		Storage_ = storage;
		if (Storage_)
		{
			connect (Storage_,
					SIGNAL (itemAdded (int)),
					this,
					SLOT (handleItemAdded (int)));
			connect (Storage_,
					SIGNAL (itemUpdated (int)),
					this,
					SLOT (handleItemUpdated (int)));
		}

		reset ();
	}

	void StorageModel::handleItemAdded (int idx)
	{
		beginInsertRows (QModelIndex (), idx, idx);
		endInsertRows ();
	}

	void StorageModel::handleItemUpdated (int idx)
	{
		emit dataChanged (index (idx, 0), index (idx, Columns::MAX - 1));
	}
}
}
