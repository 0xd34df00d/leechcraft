/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemselectiontracker.h"
#include <QAbstractItemView>
#include "interfaces/aggregator/iitemsmodel.h"

namespace LC::Aggregator
{
	ItemSelectionTracker::ItemSelectionTracker (QAbstractItemView& view, ItemActions& actions, QObject *parent)
	: QObject { parent }
	{
		const auto sm = view.selectionModel ();

		const auto commonHandler = [=, &actions]
		{
			const auto& rows = sm->selectedRows ();
			actions.HandleSelectionChanged (rows);
			if (EmitRefreshes_)
				emit refreshItemDisplay ();

			SaveLastSelection (rows);
		};

		connect (sm,
				&QItemSelectionModel::selectionChanged,
				this,
				commonHandler);
		connect (view.model (),
				&QAbstractItemModel::modelReset,
				this,
				commonHandler);

		connect (view.model (),
				&QAbstractItemModel::dataChanged,
				this,
				[=, &actions] (const QModelIndex& from, const QModelIndex& to)
				{
					for (int row = from.row (); row <= to.row (); ++row)
					{
						const auto changedItemId = from.siblingAtRow (row).data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
						if (LastSelection_.contains (changedItemId))
						{
							actions.HandleSelectionChanged (sm->selectedRows ());
							return;
						}
					}
				});
	}

	void ItemSelectionTracker::SetItemDependsOnSelection (bool depends)
	{
		EmitRefreshes_ = depends;
	}

	void ItemSelectionTracker::SaveLastSelection (const QModelIndexList& rows)
	{
		LastSelection_.clear ();
		for (const auto& row : rows)
			LastSelection_ << row.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
	}
}
