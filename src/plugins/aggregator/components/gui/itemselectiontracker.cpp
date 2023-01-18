/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemselectiontracker.h"
#include <QAbstractItemView>

namespace LC::Aggregator
{
	ItemSelectionTracker::ItemSelectionTracker (QAbstractItemView& view, ItemActions& actions, QObject *parent)
	: QObject { parent }
	{
		const auto sm = view.selectionModel ();
		const auto commonHandler = [this, sm, &actions]
		{
			actions.HandleSelectionChanged (sm->selectedRows ());
			if (EmitRefreshes_)
				emit refreshItemDisplay ();
		};

		connect (sm,
				&QItemSelectionModel::selectionChanged,
				this,
				commonHandler);
		connect (view.model (),
				&QAbstractItemModel::modelReset,
				this,
				commonHandler);
	}

	void ItemSelectionTracker::SetItemDependsOnSelection (bool depends)
	{
		EmitRefreshes_ = depends;
	}
}
