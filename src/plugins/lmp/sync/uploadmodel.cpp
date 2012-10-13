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

#include "uploadmodel.h"
#include <QtDebug>

namespace LeechCraft
{
namespace LMP
{
	UploadModel::UploadModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
	}

	QSet<QPersistentModelIndex> UploadModel::GetSelectedIndexes () const
	{
		return SourceIndexes_;
	}

	Qt::ItemFlags UploadModel::flags (const QModelIndex& idx) const
	{
		return QSortFilterProxyModel::flags (idx) | Qt::ItemIsUserCheckable;
	}

	QVariant UploadModel::data (const QModelIndex& idx, int role) const
	{
		const auto& var = QSortFilterProxyModel::data (idx, role);
		if (role != Qt::CheckStateRole)
			return var;

		return SourceIndexes_.contains (mapToSource (idx)) ?
				Qt::Checked :
				Qt::Unchecked;
	}

	bool UploadModel::setData (const QModelIndex& idx, const QVariant& data, int role)
	{
		if (role != Qt::CheckStateRole)
			return false;

		if (data.toBool ())
		{
			SourceIndexes_ << mapToSource (idx);
			emit dataChanged (idx, idx);
		}
		else
		{
			auto parent = idx;
			while (parent.isValid ())
			{
				SourceIndexes_.remove (mapToSource (parent));
				emit dataChanged (parent, parent);
				parent = parent.parent ();
			}
		}

		for (int i = 0, rc = rowCount (idx); i < rc; ++i)
			setData (index (i, 0, idx), data, Qt::CheckStateRole);

		return true;
	}

	bool UploadModel::filterAcceptsRow (int, const QModelIndex&) const
	{
		return true;
	}
}
}
