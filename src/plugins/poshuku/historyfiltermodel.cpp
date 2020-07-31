/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historyfiltermodel.h"
#include "historymodel.h"

namespace LC
{
namespace Poshuku
{
	HistoryFilterModel::HistoryFilterModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
	}
	
	bool HistoryFilterModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		if (sourceModel ()->rowCount (sourceModel ()->index (row, 0, parent)))
			return true;
		
		const auto& filter = filterRegExp ().pattern ();
		if (filter.isEmpty ())
			return true;
		
		auto source = sourceModel ();
		auto contains = [&filter, source, row, parent] (HistoryModel::Columns col)
		{
			return source->index (row, col, parent).data ()
					.toString ().contains (filter, Qt::CaseInsensitive);
		};
		return contains (HistoryModel::ColumnTitle) || contains (HistoryModel::ColumnURL);
	}
}
}
