/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsfiltermodel.h"
#include <QtDebug>
#include <util/sll/containerconversions.h>
#include "interfaces/aggregator/iitemsmodel.h"
#include "itemswidget.h"
#include "xmlsettingsmanager.h"
#include "storagebackendmanager.h"

namespace LC
{
namespace Aggregator
{
	ItemsFilterModel::ItemsFilterModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);

		XmlSettingsManager::Instance ()->RegisterObject ("UnreadOnTop", this,
				[this] (bool unreadOnTop)
				{
					UnreadOnTop_ = unreadOnTop;
					invalidateFilter ();
				});
	}

	void ItemsFilterModel::SetItemsWidget (ItemsWidget *w)
	{
		ItemsWidget_ = w;
	}

	void ItemsFilterModel::SetHideRead (bool hide)
	{
		HideRead_ = hide;
		invalidate ();
	}

	void ItemsFilterModel::SetItemTags (QList<ITagsManager::tag_id> tags)
	{
		if (tags.isEmpty ())
			TaggedItems_.clear ();
		else
		{
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			TaggedItems_ = Util::AsSet (sb->GetItemsForTag (tags.takeFirst ()));

			for (const auto& tag : tags)
			{
				const auto& set = Util::AsSet (sb->GetItemsForTag (tag));
				TaggedItems_.intersect (set);
				if (TaggedItems_.isEmpty ())
					TaggedItems_ << -1;
			}
		}

		invalidate ();
	}

	bool ItemsFilterModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		const auto& index = sourceModel ()->index (sourceRow, 0, sourceParent);
		if (HideRead_ &&
				index.data (IItemsModel::ItemRole::IsRead).toBool () &&
				ItemsWidget_->GetUnfilteredSelectedIndex () != index)
			return false;

		if (!ItemCategories_.isEmpty ())
		{
			const auto& itemCategories = index.data (IItemsModel::ItemRole::ItemCategories).toStringList ();

			const bool categoryFound = itemCategories.isEmpty () ?
					true :
					std::any_of (itemCategories.begin (), itemCategories.end (),
							[this] (const QString& cat) { return ItemCategories_.contains (cat); });
			if (!categoryFound)
				return false;
		}

		if (!TaggedItems_.isEmpty ())
		{
			const auto itemId = index.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
			if (!TaggedItems_.contains (itemId))
				return false;
		}

		return QSortFilterProxyModel::filterAcceptsRow (sourceRow, sourceParent);
	}

	bool ItemsFilterModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		if (left.column () == 1 &&
				right.column () == 1 &&
				UnreadOnTop_ &&
				!HideRead_)
		{
			const bool lr = left.data (IItemsModel::ItemRole::IsRead).toBool ();
			const bool rr = right.data (IItemsModel::ItemRole::IsRead).toBool ();
			if (lr && !rr)
				return true;
			else if (lr == rr)
				return QSortFilterProxyModel::lessThan (left, right);
			else
				return false;
		}
		return QSortFilterProxyModel::lessThan (left, right);
	}

	void ItemsFilterModel::categorySelectionChanged (const QStringList& categories)
	{
		ItemCategories_ = Util::AsSet (categories);
		invalidateFilter ();
	}
}
}
