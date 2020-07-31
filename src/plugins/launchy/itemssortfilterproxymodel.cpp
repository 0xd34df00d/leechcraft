/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemssortfilterproxymodel.h"
#include <QtDebug>
#include <QTimer>
#include "modelroles.h"

namespace LC
{
namespace Launchy
{
	ItemsSortFilterProxyModel::ItemsSortFilterProxyModel (QAbstractItemModel *source, QObject *parent)
	: RoleNamesMixin<QSortFilterProxyModel> (parent)
	{
		setDynamicSortFilter (true);
		setSourceModel (source);
		setRoleNames (source->roleNames ());
		sort (0, Qt::AscendingOrder);
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

	bool ItemsSortFilterProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		if (!AppFilterText_.isEmpty () || CategoryNames_ != QStringList ("X-Recent"))
			return QSortFilterProxyModel::lessThan (left, right);

		const auto leftPos = left.data (ModelRoles::ItemRecentPos).toInt ();
		const auto rightPos = right.data (ModelRoles::ItemRecentPos).toInt ();
		return leftPos < rightPos;
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

			if (CategoryNames_.contains ("X-Recent") &&
					idx.data (ModelRoles::IsItemRecent).toBool ())
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
				checkStr (ModelRoles::ItemDescription) ||
				checkStr (ModelRoles::ItemCommand);
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
