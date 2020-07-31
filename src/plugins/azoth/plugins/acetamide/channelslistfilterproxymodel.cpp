/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelslistfilterproxymodel.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ChannelsListFilterProxyModel::ChannelsListFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);
		setSortCaseSensitivity (Qt::CaseInsensitive);
		setSortLocaleAware (true);
	}

	bool ChannelsListFilterProxyModel::filterAcceptsRow (int sourceRow,
			const QModelIndex& sourceParent) const
	{
		const QModelIndex index = sourceModel()->index (sourceRow, 0, sourceParent);

		return sourceModel ()->data (index).toString ().contains (filterRegExp ());
	}

	bool ChannelsListFilterProxyModel::lessThan (const QModelIndex &left,
			const QModelIndex &right) const
	{
		QVariant leftData = sourceModel ()->data (left);
		QVariant rightData = sourceModel ()->data (right);

		QString leftString = leftData.toString ();
		if (left.column () == 0)
			leftString = leftString.mid (1);

		QString rightString = rightData.toString ();
		if (right.column () == 0)
			rightString = rightString.mid (1);

		return QString::localeAwareCompare (leftString, rightString) > 0;
	}
}
}
}
