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
	: QSortFilterProxyModel (parent)
	, Separator_ (GetDefaultTagsSeparator ())
	{
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
		return NormalMode_ ?
				FilterNormalMode (sourceRow, index) :
				FilterTagsMode (sourceRow, index);
	}

	bool TagsFilterModel::FilterNormalMode (int sourceRow, const QModelIndex& index) const
	{
		if (index.isValid () && sourceModel ()->rowCount (index))
			return true;

		const auto& pattern = filterRegularExpression ().pattern ();
		if (pattern.isEmpty ())
			return true;

		for (int i = 0, cc = sourceModel ()->columnCount (index); i < cc; ++i)
		{
			const auto& rowIdx = sourceModel ()->index (sourceRow, i, index);
			const auto& str = rowIdx.data ().toString ();
			if (str.contains (pattern) || filterRegularExpression ().match (str).hasMatch ())
				return true;
		}

		return false;
	}

	bool TagsFilterModel::FilterTagsMode (int sourceRow, const QModelIndex&) const
	{
		QList<QStringView> filterTags;
		const auto& pattern = filterRegularExpression ().pattern ();
		for (const auto& s : QStringView { pattern }.split (Separator_, Qt::SkipEmptyParts))
			filterTags << s.trimmed ();

		if (filterTags.isEmpty ())
			return true;

		const auto& itemTags = GetTagsForIndex (sourceRow);
		const auto hasTag = [&] (QStringView tag) { return itemTags.contains (tag); };
		switch (TagsMode_)
		{
		case TagsInclusionMode::Any:
			return std::any_of (filterTags.begin (), filterTags.end (), hasTag);
		case TagsInclusionMode::All:
			return std::all_of (filterTags.begin (), filterTags.end (), hasTag);
		}

		Util::Unreachable ();
	}
}
