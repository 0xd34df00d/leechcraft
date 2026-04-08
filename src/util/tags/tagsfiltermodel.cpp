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

	void TagsFilterModel::SetTagsRole (int role)
	{
		beginFilterChange ();
		TagsRole_ = role;
		endFilterChange (Direction::Rows);
	}

	void TagsFilterModel::SetSeparator (const QString& separator)
	{
		beginFilterChange ();
		Separator_ = separator;
		endFilterChange (Direction::Rows);
	}

	void TagsFilterModel::SetTagsInclusionMode (TagsFilterModel::TagsInclusionMode mode)
	{
		beginFilterChange ();
		TagsMode_ = mode;
		endFilterChange (Direction::Rows);
	}

	void TagsFilterModel::SetTagsMode (bool tags)
	{
		beginFilterChange ();
		NormalMode_ = !tags;
		endFilterChange (Direction::Rows);
	}

	bool TagsFilterModel::filterAcceptsRow (int sourceRow, const QModelIndex& index) const
	{
		if (NormalMode_)
			return FixedStringFilterProxyModel::filterAcceptsRow (sourceRow, index);

		return FilterTagsMode (sourceRow, index);
	}

	QStringList TagsFilterModel::GetTagsForIndex (int row) const
	{
		if (TagsRole_ < 0)
			throw std::runtime_error { "no tags role for the default TagsFilterModel::GetTagsForIndex() implementation" };

		const auto model = sourceModel ();
		if (!model)
			return {};

		return model->index (row, 0).data (TagsRole_).toStringList ();
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
