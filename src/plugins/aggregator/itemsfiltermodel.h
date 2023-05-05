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

namespace LC::Aggregator
{
	class ItemsWidget;

	class ItemsFilterModel : public QSortFilterProxyModel
	{
		bool HideRead_ = false;
		bool UnreadOnTop_ = false;
		QSet<QString> ItemCategories_;
		ItemsWidget *ItemsWidget_ = nullptr;
		QSet<IDType_t> TaggedItems_;
	public:
		explicit ItemsFilterModel (QObject* = nullptr);

		void SetItemsWidget (ItemsWidget*);
		void SetHideRead (bool);
		void SetItemTags (QList<ITagsManager::tag_id>);

		void InvalidateCategorySelection (const QStringList&);
		void InvalidateItemsSelection ();
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
		bool lessThan (const QModelIndex&, const QModelIndex&) const override;
	};
}
