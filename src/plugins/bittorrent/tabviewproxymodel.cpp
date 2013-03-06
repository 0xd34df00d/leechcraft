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

#include "tabviewproxymodel.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"

namespace LeechCraft
{
namespace Plugins
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
		Q_FOREACH (const auto& tagId, torrentTags)
			if (reqTags.contains (tm->GetTag (tagId)))
				return true;

		return false;
	}

	void TabViewProxyModel::setStateFilterMode (int mode)
	{
		StateFilter_ = static_cast<StateFilterMode> (mode);
		invalidateFilter ();
	}
}
}
}
