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
#include "types.h"

namespace LC::BitTorrent
{
	void TabViewProxyModel::SetStateFilterMode (int mode)
	{
		beginFilterChange ();
		StateFilter_ = static_cast<StateFilterMode> (mode);
		endFilterChange (Direction::Rows);
	}

	void TabViewProxyModel::UpdateDerivedFilter (const QString& filter)
	{
		FilterTags_ = GetProxyHolder ()->GetTagsManager ()->Split (filter);
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

		if (IsMatch (idx.data ().toString ()))
			return true;

		const auto& torrentTags = idx.data (Roles::TorrentTags).toStringList ();
		return std::ranges::any_of (FilterTags_,
				[&] (const QString& tag) { return torrentTags.contains (tag); });
	}
}
