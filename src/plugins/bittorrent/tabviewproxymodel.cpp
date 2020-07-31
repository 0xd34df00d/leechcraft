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
#include "core.h"

namespace LC
{
namespace BitTorrent
{
	TabViewProxyModel::TabViewProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	, StateFilter_ (StateFilterMode::All)
	{
	}

	bool TabViewProxyModel::filterAcceptsRow (int row, const QModelIndex&) const
	{
		const auto& idx = Core::Instance ()->index (row, Core::ColumnName);
		const auto& h = Core::Instance ()->GetTorrentHandle (idx.row ());
		const auto state = h.status ().state;

		switch (StateFilter_)
		{
		case StateFilterMode::All:
			break;
		case StateFilterMode::Downloading:
			if (state != libtorrent::torrent_status::downloading_metadata &&
					state != libtorrent::torrent_status::downloading)
				return false;
			break;
		case StateFilterMode::Seeding:
			if (state != libtorrent::torrent_status::seeding &&
					state != libtorrent::torrent_status::finished)
				return false;
			break;
		}

		const auto& pattern = filterRegExp ().pattern ();
		if (pattern.isEmpty ())
			return true;

		if (idx.data ().toString ().contains (pattern, Qt::CaseInsensitive))
			return true;

		auto tm = Core::Instance ()->GetProxy ()->GetTagsManager ();
		const auto& reqTags = tm->Split (pattern);
		const auto& torrentTags = idx.data (RoleTags).toStringList ();

		return std::any_of (torrentTags.begin (), torrentTags.end (),
				[&] (const auto& tagId) { return reqTags.contains (tm->GetTag (tagId)); });
	}

	void TabViewProxyModel::setStateFilterMode (int mode)
	{
		StateFilter_ = static_cast<StateFilterMode> (mode);
		invalidateFilter ();
	}
}
}
