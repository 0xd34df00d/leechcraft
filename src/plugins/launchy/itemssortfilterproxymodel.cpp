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

#include "itemssortfilterproxymodel.h"
#include "modelroles.h"

namespace LeechCraft
{
namespace Launchy
{
	ItemsSortFilterProxyModel::ItemsSortFilterProxyModel (QAbstractItemModel *source, QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
		setSourceModel (source);
		setRoleNames (source->roleNames ());
	}

	bool ItemsSortFilterProxyModel::filterAcceptsRow (int row, const QModelIndex&) const
	{
		const auto& idx = sourceModel ()->index (row, 0);

		if (AppFilterText_.isEmpty ())
		{
			if (CategoryNames_.isEmpty ())
				return false;

			const auto& itemCats = idx.data (ModelRoles::ItemNativeCategories).toStringList ();
			return std::find_if (CategoryNames_.begin (), CategoryNames_.end (),
					[&itemCats] (const QString& cat)
						{ return itemCats.contains (cat); }) != CategoryNames_.end ();
		}
		return false;
	}

	void ItemsSortFilterProxyModel::setCategoryNames (const QStringList& cats)
	{
		CategoryNames_ = cats;
		invalidateFilter ();
	}
}
}
