/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "hubsortfiltermodel.h"
#include <plugininterface/listmodel.h>
#include "hub.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			HubSortFilterModel::HubSortFilterModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
				setSortCaseSensitivity (Qt::CaseInsensitive);
				setDynamicSortFilter (true);
			}

			bool HubSortFilterModel::lessThan (const QModelIndex& left,
					const QModelIndex& right) const
			{
				Hub::UserInfo* uiLeft = static_cast<Hub::UserInfo*> (left
							.data (Util::ListModel::RolePointer).value<void*> ());
				Hub::UserInfo* uiRight = static_cast<Hub::UserInfo*> (right
							.data (Util::ListModel::RolePointer).value<void*> ());
				switch (left.column ())
				{
					case Hub::CNick:
						{
							bool lio = uiLeft->GetIdentity ().isOp ();
							bool rio = uiRight->GetIdentity ().isOp ();

							return (!lio && rio) || !(lio && !rio);
						}
					case Hub::CShared:
						return uiLeft->GetIdentity ().getBytesShared () <
							uiRight->GetIdentity ().getBytesShared ();
					default:
						return QSortFilterProxyModel::lessThan (left, right);
				}
			}
		};
	};
};

