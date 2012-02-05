/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "readchannelsfilter.h"
#include "aggregatorapp.h"

namespace LeechCraft
{
namespace Aggregator
{
namespace WebAccess
{
	ReadChannelsFilter::ReadChannelsFilter (Wt::WObject *parent)
	: WSortFilterProxyModel (parent)
	, HideRead_ (true)
	{
		setDynamicSortFilter (true);
	}

	void ReadChannelsFilter::SetHideRead (bool hide)
	{
		HideRead_ = hide;
		sort (0);
	}

	bool ReadChannelsFilter::filterAcceptRow (int row, const Wt::WModelIndex& parent) const
	{
		if (HideRead_)
		{
			auto idx = sourceModel ()->index (row, 0, parent);
			if (idx.isValid ())
			{
				const auto data = idx.data (AggregatorApp::ChannelRole::UnreadCount);
				return boost::any_cast<int> (data) > 0;
			}
		}
		return Wt::WSortFilterProxyModel::filterAcceptRow (row, parent);
	}
}
}
}
