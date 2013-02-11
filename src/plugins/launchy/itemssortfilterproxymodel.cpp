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
#include <QtDebug>
#include <QTimer>
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

	QString ItemsSortFilterProxyModel::GetAppFilterText () const
	{
		return AppFilterText_;
	}

	void ItemsSortFilterProxyModel::SetAppFilterText (const QString& text)
	{
		AppFilterText_ = text;
		QTimer::singleShot (0,
				this,
				SLOT (invalidateFilterSlot ()));
	}

	bool ItemsSortFilterProxyModel::filterAcceptsRow (int row, const QModelIndex&) const
	{
		const auto& idx = sourceModel ()->index (row, 0);

		if (AppFilterText_.isEmpty ())
		{
			if (CategoryNames_.isEmpty ())
				return false;

			if (CategoryNames_.contains ("X-Favorites") &&
					idx.data (ModelRoles::IsItemFavorite).toBool ())
				return true;

			const auto& itemCats = idx.data (ModelRoles::ItemNativeCategories).toStringList ();
			return std::find_if (CategoryNames_.begin (), CategoryNames_.end (),
					[&itemCats] (const QString& cat)
						{ return itemCats.contains (cat); }) != CategoryNames_.end ();
		}

		auto checkStr = [&idx, this] (int role)
		{
			return idx.data (role).toString ().contains (AppFilterText_, Qt::CaseInsensitive);
		};

		return checkStr (ModelRoles::ItemName) ||
				checkStr (ModelRoles::ItemDescription);
	}

	void ItemsSortFilterProxyModel::setCategoryNames (const QStringList& cats)
	{
		CategoryNames_ = cats;
		invalidateFilter ();
	}

	void ItemsSortFilterProxyModel::invalidateFilterSlot ()
	{
		invalidateFilter ();
	}
}
}
