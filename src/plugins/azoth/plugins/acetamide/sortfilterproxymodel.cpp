/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "sortfilterproxymodel.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	SortFilterProxyModel::SortFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
	}

	bool SortFilterProxyModel::filterAcceptsRow (int sourceRow,
			const QModelIndex& sourceParent) const
	{
			QModelIndex index0 = sourceModel ()->
					index (sourceRow, 1, sourceParent);
			QModelIndex index1 = sourceModel ()->
					index (sourceRow, 2, sourceParent);
			QModelIndex index2 = sourceModel ()->
					index (sourceRow, 3, sourceParent);

			return (sourceModel ()->data (index0).toString ().contains (filterRegExp ())
					|| sourceModel ()->data (index1).toString ().contains (filterRegExp ())
				|| sourceModel ()->data (index2).toString ().contains (filterRegExp ()));
	}
}
}
}