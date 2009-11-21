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

#include <QStringList>
#include <QtDebug>
#include "filtermodel.h"
#include "core.h"
#include "tagsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			FilterModel::FilterModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			, NormalMode_ (true)
			{
			}

			void FilterModel::SetTagsMode (bool tagsMode)
			{
				NormalMode_ = !tagsMode;
				invalidateFilter ();
			}

			bool FilterModel::filterAcceptsRow (int source_row, const QModelIndex& source_parent) const
			{
				if (NormalMode_)
					return QSortFilterProxyModel::filterAcceptsRow (source_row, source_parent);
				else
				{
					QStringList itemTags = Core::Instance ().GetTagsForIndex (source_row, sourceModel ()),
								filterTags = TagsManager::Instance ().Split (filterRegExp ().pattern ());
					if (!filterTags.size () || !itemTags.size ())
						return true;

					for (int i = 0; i < filterTags.size (); ++i)
					{
						bool found = false;
						for (int j = 0; j < itemTags.size (); ++j)
							if (itemTags.at (j).contains (filterTags.at (i)))
							{
								found = true;
								break;
							}
						if (!found)
							return false;
					}
					return true;
				}
			}
		};
	};
};

