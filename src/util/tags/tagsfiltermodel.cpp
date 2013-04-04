/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

namespace LeechCraft
{
namespace Util
{
	TagsFilterModel::TagsFilterModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	, NormalMode_ (true)
	, Separator_ (";")
	, TagsMode_ (TagsInclusionMode::All)
	{
	}

	void TagsFilterModel::SetSeparator (const QString& separator)
	{
		Separator_ = separator;

		if (dynamicSortFilter ())
			invalidateFilter ();
	}

	void TagsFilterModel::SetTagsInclusionMode (TagsFilterModel::TagsInclusionMode mode)
	{
		TagsMode_ = mode;

		if (dynamicSortFilter ())
			invalidateFilter ();
	}

	void TagsFilterModel::setTagsMode (bool tags)
	{
		NormalMode_ = !tags;

		if (dynamicSortFilter ())
			invalidateFilter ();
	}

	void TagsFilterModel::enableTagsMode ()
	{
		setTagsMode (true);
	}

	void TagsFilterModel::disableTagsMode ()
	{
		setTagsMode (false);
	}

	bool TagsFilterModel::filterAcceptsRow (int source_row, const QModelIndex& index) const
	{
		if (NormalMode_)
			return (index.isValid () && index.model ()->rowCount (index)) ?
				true :
				QSortFilterProxyModel::filterAcceptsRow (source_row, index);
		else
		{
			QStringList filterTags;
			const auto& pattern = filterRegExp ().pattern ();
			for (const auto& s : pattern.split (Separator_, QString::SkipEmptyParts))
				filterTags << s.trimmed ();

			if (!filterTags.size ())
				return true;

			const auto& itemTags = GetTagsForIndex (source_row);
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
				else if (TagsMode_ == TagsInclusionMode::Any)
					return true;
			}
			return true;
		}
	}
}
}
