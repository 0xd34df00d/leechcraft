/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "sortfilterproxymodel.h"
#include "core.h"
#include "interfaces/iclentry.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			SortFilterProxyModel::SortFilterProxyModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
			}

			namespace
			{
				Core::CLEntryType GetType (const QModelIndex& idx)
				{
					return idx.data (Core::CLREntryType).value<Core::CLEntryType> ();
				}

				Plugins::ICLEntry* GetEntry (const QModelIndex& idx)
				{
					return qobject_cast<Plugins::ICLEntry*> (idx
								.data (Core::CLREntryObject).value<QObject*> ());
				}
			}

			bool SortFilterProxyModel::lessThan (const QModelIndex& right,
					const QModelIndex& left) const			// sort in reverse order ok
			{
				if (GetType (left) != Core::CLETContact ||
						GetType (right) != Core::CLETContact)
					return QSortFilterProxyModel::lessThan (left, right);

				Plugins::ICLEntry *lE = GetEntry (left);
				Plugins::ICLEntry *rE = GetEntry (right);

				Plugins::State lState = lE->GetStatus ().State_;
				Plugins::State rState = rE->GetStatus ().State_;
				if (lState == rState)
					return lE->GetEntryName () < rE->GetEntryName ();
				else
					return IsLess (lState, rState);
			}
		}
	}
}
