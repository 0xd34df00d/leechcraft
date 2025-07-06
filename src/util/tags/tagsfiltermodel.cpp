/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsfiltermodel.h"
#include <QRegularExpression>
#include <QStringList>
#include <util/sll/unreachable.h>
#include "util.h"

namespace LC::Util
{
	TagsFilterModel::TagsFilterModel (QObject *parent)
	: FixedStringFilterProxyModel { parent }
	, Separator_ { GetDefaultTagsSeparator () }
	{
	}

	void TagsFilterModel::SetFilterString (const QString& string)
	{
		FilterTags_.clear ();
		for (const auto& s : string.split (Separator_, Qt::SkipEmptyParts))
			FilterTags_ << s.trimmed ();

		FixedStringFilterProxyModel::SetFilterString (string);
	}

	void TagsFilterModel::SetSeparator (const QString& separator)
	{
		Separator_ = separator;
		invalidateFilter ();
	}

	void TagsFilterModel::SetTagsInclusionMode (TagsFilterModel::TagsInclusionMode mode)
	{
		TagsMode_ = mode;
		invalidateFilter ();
	}

	void TagsFilterModel::SetTagsMode (bool tags)
	{
		NormalMode_ = !tags;
		invalidateFilter ();
	}

	bool TagsFilterModel::filterAcceptsRow (int sourceRow, const QModelIndex& index) const
	{
		if (NormalMode_)
			return FixedStringFilterProxyModel::filterAcceptsRow (sourceRow, index);

		return FilterTagsMode (sourceRow, index);
	}

	bool TagsFilterModel::FilterTagsMode (int sourceRow, const QModelIndex&) const
	{
		if (FilterTags_.isEmpty ())
			return true;

		const auto& itemTags = GetTagsForIndex (sourceRow);
		const auto hasTag = [&] (const QString& tag) { return itemTags.contains (tag); };
		switch (TagsMode_)
		{
		case TagsInclusionMode::Any:
			return std::ranges::any_of (FilterTags_, hasTag);
		case TagsInclusionMode::All:
			return std::ranges::all_of (FilterTags_, hasTag);
		}

		Unreachable ();
	}
}
