/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "stringfiltermodel.h"
#include <QSet>
#include "packagesmodel.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			StringFilterModel::StringFilterModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
			}
			
			bool StringFilterModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
			{
				if (QSortFilterProxyModel::filterAcceptsRow (sourceRow, sourceParent))
					return true;
				
				const QString& filterString = filterRegExp ().pattern ();
				const QModelIndex& idx = sourceModel ()->index (sourceRow, 0, sourceParent);
				
				if (sourceModel ()->data (idx, PackagesModel::PMRShortDescription)
						.toString ().contains (filterString, Qt::CaseInsensitive))
					return true;
				
				const QSet<QString>& tags = QSet<QString>::fromList (sourceModel ()->
								data (idx, PackagesModel::PMRTags).toStringList ());
				const QStringList& queryList = Core::Instance ().GetProxy ()->
						GetTagsManager ()->Split (filterString);
				QSet<QString> userDefined = QSet<QString>::fromList (queryList);
				
				return !userDefined.intersect (tags).empty ();
			}
		}
	}
}
