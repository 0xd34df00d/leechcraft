/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsfiltermodel.h"
#include <algorithm>
#include <QIdentityProxyModel>
#include <QtDebug>
#include <util/sll/containerconversions.h>
#include "interfaces/aggregator/iitemsmodel.h"
#include "itemswidget.h"
#include "xmlsettingsmanager.h"
#include "storagebackendmanager.h"

namespace LC::Aggregator
{
	class ItemsFilterModel::SelectedIdProxyModel : public QIdentityProxyModel
	{
		QSet<IDType_t> Selections_;
		IItemsModel& SourceModel_;
	public:
		constexpr static auto IsSelectedRole = IItemsModel::MaxItemRole + 1;

		explicit SelectedIdProxyModel (IItemsModel& sourceModel)
		: SourceModel_ { sourceModel }
		{
			setSourceModel (&SourceModel_.GetQModel ());
		}

		void SetSelections (const QSet<IDType_t>& selections)
		{
			if (Selections_ == selections)
				return;

			EmitByIds (std::exchange (Selections_, {}));
			Selections_ = selections;
			EmitByIds (Selections_);
		}

		QVariant data (const QModelIndex& index, int role) const override
		{
			if (role != IsSelectedRole)
				return QIdentityProxyModel::data (index, role);

			const auto id = index.data (IItemsModel::ItemId).value<IDType_t> ();
			return Selections_.contains (id);
		}
	private:
		void EmitByIds (const QSet<IDType_t>& ids)
		{
			const auto lastColumn = sourceModel ()->columnCount () - 1;
			for (const auto& idx : SourceModel_.FindItems (ids))
				emit dataChanged (idx.siblingAtColumn (0), idx.siblingAtColumn (lastColumn), { IsSelectedRole });
		}
	};

	ItemsFilterModel::ItemsFilterModel (IItemsModel& sourceModel, QObject *parent)
	: QSortFilterProxyModel { parent }
	, SelectedIdProxyModel_ { std::make_unique<SelectedIdProxyModel> (sourceModel) }
	, HideRead_ { XmlSettingsManager::Instance ().Property ("HideReadItems", false).toBool () }
	{
		QSortFilterProxyModel::setSourceModel (SelectedIdProxyModel_.get ());

		setDynamicSortFilter (true);

		XmlSettingsManager::Instance ().RegisterObject ("UnreadOnTop", this,
				[this] (bool unreadOnTop)
				{
					UnreadOnTop_ = unreadOnTop;
					invalidate ();
				});
	}

	ItemsFilterModel::~ItemsFilterModel () = default;

	void ItemsFilterModel::SetHideRead (bool hide)
	{
		HideRead_ = hide;
		XmlSettingsManager::Instance ().setProperty ("HideReadItems", hide);
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

	void ItemsFilterModel::InvalidateCategorySelection (const QStringList& categoriesList)
	{
		auto categories = Util::AsSet (categoriesList);
		if (categories == ItemCategories_)
			return;

		ItemCategories_ = std::move (categories);
		invalidateFilter ();
	}

	void ItemsFilterModel::InvalidateItemsSelection (const QSet<IDType_t>& selections)
	{
		SelectedIdProxyModel_->SetSelections (selections);
	}

	void ItemsFilterModel::setSourceModel (QAbstractItemModel*)
	{
		throw std::logic_error { "ItemsFilterModel::setSourceModel is not supposed to be called" };
	}

	bool ItemsFilterModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		const auto& index = sourceModel ()->index (sourceRow, 0, sourceParent);
		const auto itemId = index.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
		if (HideRead_ &&
				index.data (IItemsModel::ItemRole::IsRead).toBool () &&
				!index.data (SelectedIdProxyModel::IsSelectedRole).toBool ())
			return false;

		if (!ItemCategories_.isEmpty ())
		{
			const auto& itemCategories = index.data (IItemsModel::ItemRole::ItemCategories).toStringList ();

			const bool categoryFound = itemCategories.isEmpty () ||
					std::ranges::any_of (itemCategories, [this] (const QString& cat) { return ItemCategories_.contains (cat); });
			if (!categoryFound)
				return false;
		}

		if (!TaggedItems_.isEmpty () && !TaggedItems_.contains (itemId))
			return false;

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
			if (lr == rr)
				return QSortFilterProxyModel::lessThan (left, right);

			return false;
		}
		return QSortFilterProxyModel::lessThan (left, right);
	}
}
