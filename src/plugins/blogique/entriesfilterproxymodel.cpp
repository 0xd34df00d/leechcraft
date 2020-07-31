/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entriesfilterproxymodel.h"

namespace LC
{
namespace Blogique
{
	EntriesFilterProxyModel::EntriesFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
	}

	bool EntriesFilterProxyModel::filterAcceptsRow (int sourceRow,
			const QModelIndex& sourceParent) const
	{
		const QModelIndex& index = sourceModel ()->index (sourceRow, 1, sourceParent);
		return sourceModel ()->data (index).toString ().contains (filterRegExp ());
	}
}
}
