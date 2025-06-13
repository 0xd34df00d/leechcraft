/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabviewproxymodel.h"
#include <algorithm>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/structures.h>
#include "types.h"

namespace LC::BitTorrent
{
	void TabViewProxyModel::SetStateFilterMode (int mode)
	{
		StateFilter_ = static_cast<StateFilterMode> (mode);
		invalidateFilter ();
	}

	bool TabViewProxyModel::filterAcceptsRow (int row, const QModelIndex&) const
	{
		const auto& idx = sourceModel ()->index (row, Columns::ColumnName, {});

		switch (StateFilter_)
		{
		case StateFilterMode::All:
			break;
		case StateFilterMode::Downloading:
			if (!idx.data (Roles::IsLeeching).toBool ())
				return false;
			break;
		case StateFilterMode::Seeding:
			if (!idx.data (Roles::IsSeeding).toBool ())
				return false;
			break;
		}

		if (FilterFixedString_.isEmpty ())
			return true;

		if (idx.data ().toString ().contains (FilterFixedString_, Qt::CaseInsensitive))
			return true;

		auto tm = GetProxyHolder ()->GetTagsManager ();
		const auto& reqTags = tm->Split (FilterFixedString_);
		const auto& torrentTags = idx.data (RoleTags).toStringList ();

		return std::ranges::any_of (torrentTags,
				[&] (const auto& tagId) { return reqTags.contains (tm->GetTag (tagId)); });
	}
}
