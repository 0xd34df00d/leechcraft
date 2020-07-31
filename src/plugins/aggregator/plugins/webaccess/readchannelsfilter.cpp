/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "readchannelsfilter.h"
#include <QtDebug>
#include "aggregatorapp.h"

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	ReadChannelsFilter::ReadChannelsFilter ()
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
				try
				{
					return Wt::cpp17::any_cast<int> (data) > 0;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "cannot get unread count"
							<< e.what ()
							<< "; stored:"
							<< data.type ().name ();
					return true;
				}
			}
		}
		return Wt::WSortFilterProxyModel::filterAcceptRow (row, parent);
	}
}
}
}
