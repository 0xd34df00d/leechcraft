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

#include "flattenfiltermodel.h"

namespace LeechCraft
{
namespace Util
{
	FlattenFilterModel::FlattenFilterModel (QObject *parent)
	: QAbstractItemModel (parent)
	, Source_ (0)
	{
	}

	QModelIndex FlattenFilterModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (parent.isValid ())
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex FlattenFilterModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int FlattenFilterModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : SourceIndexes_.size ();
	}

	int FlattenFilterModel::columnCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : 1;
	}

	QVariant FlattenFilterModel::data (const QModelIndex& index, int role) const
	{
		return SourceIndexes_.value (index.row ()).data (role);
	}

	void FlattenFilterModel::SetSource (QAbstractItemModel *model)
	{
		if (Source_)
		{
			disconnect (Source_,
					SIGNAL (rowsInserted (QModelIndex, int, int)),
					this,
					SLOT (handleRowsInserted (QModelIndex, int, int)));
			disconnect (Source_,
					SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
					this,
					SLOT (handleRowsAboutRemoved (QModelIndex, int, int)));
			disconnect (Source_,
					SIGNAL (dataChanged (QModelIndex, QModelIndex)),
					this,
					SLOT (handleDataChanged (QModelIndex, QModelIndex)));
		}

		SourceIndexes_.clear ();
		Source_ = model;
		reset ();
		connect (Source_,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)));
		connect (Source_,
				SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
				this,
				SLOT (handleRowsAboutRemoved (QModelIndex, int, int)));
		connect (Source_,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDataChanged (QModelIndex, QModelIndex)));
	}

	bool FlattenFilterModel::IsIndexAccepted (const QModelIndex&) const
	{
		return true;
	}

	void FlattenFilterModel::handleDataChanged (const QModelIndex& top, const QModelIndex& bottom)
	{
		const auto& parent = top.parent ();
		for (int i = top.row (); i <= bottom.row (); ++i)
		{
			const auto& child = Source_->index (i, 0, parent);
			const int pos = SourceIndexes_.indexOf (child);
			if (pos < 0)
				continue;

			const auto& ourIdx = index (pos, 0);
			emit dataChanged (ourIdx, ourIdx);
		}
	}

	void FlattenFilterModel::handleRowsInserted (const QModelIndex& parent, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			const auto& child = Source_->index (i, 0, parent);
			if (IsIndexAccepted (child))
			{
				beginInsertRows (QModelIndex (), SourceIndexes_.size (), SourceIndexes_.size ());
				SourceIndexes_ << child;
				endInsertRows ();
			}

			if (int rc = Source_->rowCount (child))
				handleRowsInserted (child, 0, rc - 1);
		}
	}

	void FlattenFilterModel::handleRowsAboutRemoved (const QModelIndex& parent, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			const auto& child = Source_->index (i, 0, parent);
			const int pos = SourceIndexes_.indexOf (child);

			if (pos >= 0)
			{
				beginRemoveRows (QModelIndex (), pos, pos);
				SourceIndexes_.removeAt (pos);
				endRemoveRows ();
			}

			if (int rc = Source_->rowCount (child))
				handleRowsAboutRemoved (child, 0, rc - 1);
		}
	}
}
}
