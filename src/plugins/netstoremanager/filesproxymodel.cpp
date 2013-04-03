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

#include "filesproxymodel.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	FilesProxyModel::FilesProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
		setSortCaseSensitivity (Qt::CaseInsensitive);
		setSortLocaleAware (true);
	}

	bool FilesProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		bool leftIsFolder = sourceModel ()->data (left, ListingRole::Directory).toBool ();
		bool rightIsFolder = sourceModel ()->data (right, ListingRole::Directory).toBool ();

		if (left.column () == 0 &&
				right.column () == 0)
		{
			if (sourceModel ()->data (left, ListingRole::ID).toByteArray () == "netstoremanager.item_trash")
				return true;
			if (sourceModel ()->data (right, ListingRole::ID).toByteArray () == "netstoremanager.item_trash")
				return false;

			if (leftIsFolder &&
					!rightIsFolder)
				return false;
			else if (!leftIsFolder &&
					rightIsFolder)
				return true;
			else
				return QString::localeAwareCompare (sourceModel ()->data (left).toString (),
						sourceModel ()->data (right).toString ()) > 0;
		}
		else
			return QString::localeAwareCompare (sourceModel ()->data (left).toString (),
					sourceModel ()->data (right).toString ()) > 0;
	}

}
}
