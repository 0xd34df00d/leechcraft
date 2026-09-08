/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemscategoriestracker.h"
#include <QTimer>
#include <util/models/modeliterator.h>
#include <util/sll/util.h>
#include <interfaces/aggregator/iitemsmodel.h>

namespace LC::Aggregator
{
	auto ItemsCategoriesTracker::PrepareCountsChanging ()
	{
		const auto initialSize = Counts_.size ();
		return Util::MakeScopeGuard ([&, initialSize]
				{
					if (initialSize != Counts_.size ())
						ScheduleCategoriesChangedSignal (Counts_.keys ());
				});
	}

	ItemsCategoriesTracker::ItemsCategoriesTracker (QAbstractItemModel& model)
	: QObject { &model }
	, Model_ { model }
	{
		connect (&model,
				&QAbstractItemModel::modelReset,
				this,
				[this]
				{
					if (!Counts_.isEmpty ())
					{
						Counts_.clear ();
						ScheduleCategoriesChangedSignal ({});
					}

					const auto guard = PrepareCountsChanging ();
					for (const auto& idx : Util::AllModelRows (Model_))
						for (const auto& cat : idx.data (IItemsModel::ItemCategories).value<QStringList> ())
							++Counts_ [cat];
				});
		connect (&model,
				&QAbstractItemModel::rowsInserted,
				this,
				[this] (const QModelIndex&, int start, int end)
				{
					const auto guard = PrepareCountsChanging ();
					for (const auto& idx : Util::ModelRows (Model_, start, end))
						for (const auto& cat : idx.data (IItemsModel::ItemCategories).value<QStringList> ())
							++Counts_ [cat];
				});
		connect (&model,
				&QAbstractItemModel::rowsAboutToBeRemoved,
				this,
				[this] (const QModelIndex&, int start, int end)
				{
					const auto guard = PrepareCountsChanging ();
					for (const auto& idx : Util::ModelRows (Model_, start, end))
						for (const auto& cat : idx.data (IItemsModel::ItemCategories).value<QStringList> ())
							if (!--Counts_ [cat])
								Counts_.remove (cat);
				});
	}

	void ItemsCategoriesTracker::ScheduleCategoriesChangedSignal (QList<QString> categories)
	{
		if (!ScheduledCategories_)
			QTimer::singleShot (0, this,
					[this] { emit categoriesChanged (*std::exchange (ScheduledCategories_, std::nullopt)); });

		ScheduledCategories_ = std::move (categories);
	}
}
