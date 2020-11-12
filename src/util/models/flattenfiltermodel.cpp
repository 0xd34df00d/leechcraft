/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flattenfiltermodel.h"

namespace LC::Util
{
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
			disconnect (Source_,
					nullptr,
					this,
					nullptr);

		beginResetModel ();
		SourceIndexes_.clear ();
		Source_ = model;
		endResetModel ();
		connect (Source_,
				&QAbstractItemModel::rowsInserted,
				this,
				&FlattenFilterModel::HandleRowsInserted);
		connect (Source_,
				&QAbstractItemModel::rowsAboutToBeRemoved,
				this,
				&FlattenFilterModel::HandleRowsAboutRemoved);
		connect (Source_,
				&QAbstractItemModel::dataChanged,
				this,
				&FlattenFilterModel::HandleDataChanged);
	}

	bool FlattenFilterModel::IsIndexAccepted (const QModelIndex&) const
	{
		return true;
	}

	void FlattenFilterModel::HandleDataChanged (const QModelIndex& top, const QModelIndex& bottom)
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

	void FlattenFilterModel::HandleRowsInserted (const QModelIndex& parent, int start, int end)
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
				HandleRowsInserted (child, 0, rc - 1);
		}
	}

	void FlattenFilterModel::HandleRowsAboutRemoved (const QModelIndex& parent, int start, int end)
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
				HandleRowsAboutRemoved (child, 0, rc - 1);
		}
	}
}
