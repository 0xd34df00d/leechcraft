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
#include "tagsfiltermodel.h"

using namespace LeechCraft::Util;

TagsFilterModel::TagsFilterModel (QObject *parent)
: QSortFilterProxyModel (parent)
, NormalMode_ (true)
{
}

void TagsFilterModel::setTagsMode (bool tags)
{
	NormalMode_ = !tags;
}

bool TagsFilterModel::filterAcceptsRow (int source_row, const QModelIndex& index) const
{
	if (NormalMode_)
		return (index.isValid () && index.model ()->rowCount (index)) ?
			true :
			QSortFilterProxyModel::filterAcceptsRow (source_row, index);
	else
	{
		QStringList itemTags = GetTagsForIndex (source_row);
		QStringList filterTags;
		QStringList splitted = filterRegExp ().pattern ().split (";", QString::SkipEmptyParts);
		Q_FOREACH (QString s, splitted)
			filterTags << s.trimmed ();

		if (!filterTags.size ())
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

