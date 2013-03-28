/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "channelslistfilterproxymodel.h"

namespace LeechCraft
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
			const QModelIndex& sourceParent)
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
		if(left.column () == 0)
			leftString = leftString.mid (1);

		QString rightString = rightData.toString ();
		if(right.column() == 0)
			rightString = rightString.mid (1);

		return QString::localeAwareCompare (leftString, rightString) > 0;
	}
}
}
}
