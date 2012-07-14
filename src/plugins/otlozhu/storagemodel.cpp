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
#include <QtDebug>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "todostorage.h"

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

	Qt::ItemFlags StorageModel::flags (const QModelIndex& index) const
	{
		return QAbstractItemModel::flags (index) | Qt::ItemIsEditable;
	}

	namespace
	{
		QString MakeTooltip (const TodoItem_ptr item)
		{
			QString result = "<strong>" + item->GetTitle () + "</strong><br />";
			result += StorageModel::tr ("%1% done").arg (item->GetPercentage ()) + "<br />";
			const auto& ids = item->GetTagIDs ();
			if (!ids.isEmpty ())
				result += Core::Instance ().GetProxy ()->
						GetTagsManager ()->JoinIDs (ids) + "<br />";

			const QString& comment = item->GetComment ();
			if (!comment.isEmpty ())
				result += comment;

			return result;
		}
	}

	QVariant StorageModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		const auto item = Storage_->GetItemAt (index.row ());
		if (role == Roles::ItemID)
			return item->GetID ();
		else if (role == Roles::ItemTitle)
			return item->GetTitle ();
		else if (role == Roles::ItemTags)
			return item->GetTagIDs ();
		else if (role == Roles::ItemProgress)
			return item->GetPercentage ();
		else if (role == Roles::ItemComment)
			return item->GetComment ();
		else if (role == Roles::ItemDueDate)
			return item->GetDueDate ();
		else if (role == Qt::ToolTipRole)
			return MakeTooltip (item);
		else if (role != Qt::DisplayRole &&
					role != Qt::EditRole)
			return QVariant ();

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
			if (role == Qt::DisplayRole)
				return date.isNull () ? QVariant (tr ("not set")) : QVariant (date);
			else
				return date.isNull () ? QDateTime::currentDateTime () : date;
		}
		case Columns::Created:
			return item->GetCreatedDate ();
		case Columns::Percentage:
			return item->GetPercentage ();
		default:
			return QVariant ();
		}
	}

	bool StorageModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (!index.isValid ())
			return false;

		auto item = Storage_->GetItemAt (index.row ());
		bool updated = false;

		if (role == Roles::ItemProgress)
		{
			item->SetPercentage (value.toInt ());
			updated = true;
		}
		else if (role == Roles::ItemComment)
		{
			item->SetComment (value.toString ());
			updated = true;
		}
		else if (role == Roles::ItemDueDate)
		{
			item->SetDueDate (value.toDateTime ());
			updated = true;
		}
		else if (role == Qt::EditRole)
			switch (index.column ())
			{
			case Columns::Title:
				item->SetTitle (value.toString ());
				updated = true;
				break;
			case Columns::Percentage:
				item->SetPercentage (value.toInt ());
				updated = true;
				break;
			case Columns::DueDate:
				item->SetDueDate (value.toDateTime ());
				updated = true;
				break;
			case Columns::Tags:
			{
				auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
				item->SetTagIDs (tm->SplitToIDs (value.toString ()));
				updated = true;
				break;
			}
			default:
				qDebug () << Q_FUNC_INFO << index.column () << value;
				break;
			}

		if (updated)
			Storage_->HandleUpdated (item);

		return updated;
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
			connect (Storage_,
					SIGNAL (itemRemoved (int)),
					this,
					SLOT (handleItemRemoved (int)));
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

	void StorageModel::handleItemRemoved (int idx)
	{
		beginRemoveRows (QModelIndex (), idx, idx);
		endRemoveRows ();
	}
}
}
