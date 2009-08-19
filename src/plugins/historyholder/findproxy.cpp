/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "findproxy.h"
#include "core.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;
using namespace LeechCraft::Plugins::HistoryHolder;

FindProxy::FindProxy (const Request& r)
{
	setSourceModel (&Core::Instance ());
	setDynamicSortFilter (true);

	setFilterCaseSensitivity (r.CaseSensitive_ ?
			Qt::CaseSensitive : Qt::CaseInsensitive);

	switch (r.Type_)
	{
		case Request::RTWildcard:
			setFilterWildcard (r.String_);
			break;
		case Request::RTRegexp:
			setFilterRegExp (r.String_);
			break;
		default:
			setFilterFixedString (r.String_);
			if (r.Type_ == Request::RTTag)
				setTagsMode (true);
			break;
	}
}

QAbstractItemModel* FindProxy::GetModel ()
{
	return this;
}

QStringList FindProxy::GetTagsForIndex (int row) const
{
	return sourceModel ()->data (sourceModel ()->
			index (row, 0), RoleTags).toStringList ();
}

