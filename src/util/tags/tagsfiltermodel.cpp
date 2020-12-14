/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QStringList>
#include <QtDebug>
#include "tagsfiltermodel.h"
#include "util.h"

namespace LC
{
namespace Util
{
	TagsFilterModel::TagsFilterModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	, NormalMode_ (true)
	, Separator_ (GetDefaultTagsSeparator ())
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

	void TagsFilterModel::SetTagsMode (bool tags)
	{
		NormalMode_ = !tags;

		if (dynamicSortFilter ())
			invalidateFilter ();
	}

	bool TagsFilterModel::filterAcceptsRow (int source_row, const QModelIndex& index) const
	{
		if (NormalMode_)
		{
			if (index.isValid () && sourceModel ()->rowCount (index))
				return true;

			const auto& pattern = filterRegExp ().pattern ();
			if (pattern.isEmpty ())
				return true;

			for (int i = 0, cc = sourceModel ()->columnCount (index); i < cc; ++i)
			{
				const auto& rowIdx = sourceModel ()->index (source_row, i, index);
				const auto& str = rowIdx.data ().toString ();
				if (str.contains (pattern) || filterRegExp ().exactMatch (str))
					return true;
			}

			return false;
		}
		else
		{
			QStringList filterTags;
			const auto& pattern = filterRegExp ().pattern ();
			for (const auto& s : pattern.split (Separator_, Qt::SkipEmptyParts))
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
