/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagschange.h"
#include <algorithm>

namespace LC::Util
{
	namespace
	{
		QStringList Difference (const QStringList& from, const QStringList& subtracted)
		{
			QStringList result;
			for (const auto& tag : from)
				if (!subtracted.contains (tag) && !result.contains (tag))
					result << tag;
			return result;
		}

		bool IsSubset (const QStringList& sub, const QStringList& of)
		{
			return std::ranges::all_of (sub, [&of] (const QString& tag) { return of.contains (tag); });
		}
	}

	QStringList TagsChange::Added () const
	{
		return Difference (After_, Before_);
	}

	QStringList TagsChange::Removed () const
	{
		return Difference (Before_, After_);
	}

	bool TagsChange::IsEmpty () const
	{
		return IsSubset (After_, Before_) && IsSubset (Before_, After_);
	}

	QStringList TagsChange::ApplyTo (QStringList tags) const
	{
		for (const auto& tag : Removed ())
			tags.removeAll (tag);

		for (const auto& tag : Added ())
			if (!tags.contains (tag))
				tags << tag;

		return tags;
	}
}
