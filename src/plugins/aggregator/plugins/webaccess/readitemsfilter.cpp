/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "readitemsfilter.h"
#include <Wt/WTimer.h>
#include "aggregatorapp.h"

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	using namespace std::chrono_literals;

	ReadItemsFilter::ReadItemsFilter ()
	{
		setDynamicSortFilter (true);
	}

	void ReadItemsFilter::SetHideRead (bool hide)
	{
		HideRead_ = hide;
		Invalidate ();
	}

	void ReadItemsFilter::SetCurrentItem (IDType_t id)
	{
		if (id == CurrentId_)
			return;

		if (Prevs_.isEmpty ())
			Wt::WTimer::singleShot (500ms, [this] { PullOnePrev (); });

		Prevs_ << CurrentId_;
		CurrentId_ = id;

		Invalidate ();
	}

	void ReadItemsFilter::ClearCurrentItem ()
	{
		SetCurrentItem (static_cast<IDType_t> (-1));
	}

	bool ReadItemsFilter::filterAcceptRow (int row, const Wt::WModelIndex& parent) const
	{
		if (HideRead_)
		{
			auto idx = sourceModel ()->index (row, 0, parent);
			if (idx.isValid ())
			{
				try
				{
					const auto idAny = idx.data (AggregatorApp::ItemRole::IID);
					const auto id = Wt::cpp17::any_cast<IDType_t> (idAny);
					if (id != CurrentId_ && !Prevs_.contains (id))
					{
						const auto data = idx.data (AggregatorApp::ItemRole::IsRead);
						if (Wt::cpp17::any_cast<bool> (data))
							return false;
					}
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "cannot get read status"
							<< e.what ();
					return true;
				}
			}
		}

		return Wt::WSortFilterProxyModel::filterAcceptRow (row, parent);
	}

	void ReadItemsFilter::PullOnePrev ()
	{
		if (Prevs_.isEmpty ())
			return;

		Prevs_.removeFirst ();
		Invalidate ();

		if (!Prevs_.isEmpty ())
			Wt::WTimer::singleShot (500ms, [this] { PullOnePrev (); });
	}

	void ReadItemsFilter::Invalidate ()
	{
		setFilterRegExp (std::make_unique<std::regex> (".*"));
	}
}
}
}
