/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>
#include <QSet>
#include <QString>
#include <interfaces/core/itagsmanager.h>
#include "common.h"

namespace LC::Util
{
	template<typename>
	class SelectionProxyModel;
}

namespace LC::Aggregator
{
	class IItemsModel;

	class ItemsFilterModel final : public QSortFilterProxyModel
	{
		QSet<QString> ItemCategories_;
		QSet<IDType_t> TaggedItems_;

		using SelectionProxy_t = Util::SelectionProxyModel<IDType_t>;
		const std::unique_ptr<SelectionProxy_t> SelectedIdProxyModel_;

		bool HideRead_ = false;
		bool UnreadOnTop_ = false;
	public:
		explicit ItemsFilterModel (IItemsModel&, QObject* = nullptr);
		~ItemsFilterModel () override;

		void SetHideRead (bool);
		void SetItemTags (QList<ITagsManager::tag_id>);

		void InvalidateCategorySelection (const QStringList&);
		void InvalidateItemsSelection (const QSet<IDType_t>&);

		void setSourceModel (QAbstractItemModel*) override;
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
		bool lessThan (const QModelIndex&, const QModelIndex&) const override;
	};
}
