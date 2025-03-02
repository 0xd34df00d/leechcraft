/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemscategoriestracker.h"
#include <util/models/modeliterator.h>
#include <util/sll/util.h>
#include <interfaces/aggregator/iitemsmodel.h>

namespace LC::Aggregator
{
	namespace
	{
		auto PrepareCountsChanging (const QHash<QString, int>& counts, ItemsCategoriesTracker& tracker)
		{
			const auto initialSize = counts.size ();

			return Util::MakeScopeGuard ([&, initialSize]
					{
						if (initialSize != counts.size ())
							emit tracker.categoriesChanged (counts.keys ());
					});
		}
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
						emit categoriesChanged ({});
					}

					const auto guard = PrepareCountsChanging (Counts_, *this);
					for (const auto& idx : Util::AllModelRows (Model_))
						for (const auto& cat : idx.data (IItemsModel::ItemCategories).value<QStringList> ())
							++Counts_ [cat];
				});
		connect (&model,
				&QAbstractItemModel::rowsInserted,
				this,
				[this] (const QModelIndex&, int start, int end)
				{
					const auto guard = PrepareCountsChanging (Counts_, *this);
					for (const auto& idx : Util::ModelRows (Model_, start, end))
						for (const auto& cat : idx.data (IItemsModel::ItemCategories).value<QStringList> ())
							++Counts_ [cat];
				});
		connect (&model,
				&QAbstractItemModel::rowsRemoved,
				this,
				[this] (const QModelIndex&, int start, int end)
				{
					const auto guard = PrepareCountsChanging (Counts_, *this);
					for (const auto& idx : Util::ModelRows (Model_, start, end))
						for (const auto& cat : idx.data (IItemsModel::ItemCategories).value<QStringList> ())
							if (!--Counts_ [cat])
								Counts_.remove (cat);
				});
	}
}
