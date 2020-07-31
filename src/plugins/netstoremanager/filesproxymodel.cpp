/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filesproxymodel.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "managertab.h"

namespace LC
{
namespace NetStoreManager
{
	FilesProxyModel::FilesProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);
		setSortCaseSensitivity (Qt::CaseInsensitive);
		setSortLocaleAware (true);
	}

	bool FilesProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		const QModelIndex& index = sourceModel ()->index (sourceRow, 0, sourceParent);
		return sourceModel ()->data (index).toString ().contains (filterRegExp ());
	}

	bool FilesProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		bool leftIsFolder = sourceModel ()->data (left, ListingRole::IsDirectory).toBool ();
		bool rightIsFolder = sourceModel ()->data (right, ListingRole::IsDirectory).toBool ();

		if (left.column () == CName)
		{
			if (sourceModel ()->data (left, ListingRole::ID).toByteArray () == "netstoremanager.item_trash" ||
					sourceModel ()->data (right, ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
				return sortOrder () == Qt::DescendingOrder ? true : false;
			if (sourceModel ()->data (right, ListingRole::ID).toByteArray () == "netstoremanager.item_trash" ||
					sourceModel ()->data (left, ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
				return sortOrder () == Qt::DescendingOrder ? false : true;

			if (leftIsFolder && !rightIsFolder)
				return false;
			else if (!leftIsFolder && rightIsFolder)
				return true;
			else
				return QString::localeAwareCompare (sourceModel ()->data (left, SRName).toString ().toLower (),
						sourceModel ()->data (right, SRName).toString ().toLower ()) > 0;
		}
		else if (left.column () == CSize)
			return sourceModel ()->data (left, SRSize).toDouble () <
					sourceModel ()->data (right, SRSize).toDouble ();
		else if (left.column () == CModify)
			return sourceModel ()->data (left, SRModifyDate).toDateTime () <
					sourceModel ()->data (right, SRModifyDate).toDateTime ();
		else
			return QString::localeAwareCompare (sourceModel ()->data (left).toString ().toLower (),
					sourceModel ()->data (right).toString ().toLower ()) > 0;
	}

}
}
