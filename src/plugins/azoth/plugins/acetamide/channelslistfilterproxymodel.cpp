/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelslistfilterproxymodel.h"

namespace LC::Azoth::Acetamide
{
	ChannelsListFilterProxyModel::ChannelsListFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);
		setSortCaseSensitivity (Qt::CaseInsensitive);
		setSortLocaleAware (true);
	}

	bool ChannelsListFilterProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		const auto& index = sourceModel()->index (sourceRow, 0, sourceParent);
		return sourceModel ()->data (index).toString ().contains (filterRegExp ());
	}
}
