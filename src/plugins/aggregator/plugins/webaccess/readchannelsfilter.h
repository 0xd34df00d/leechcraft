/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <Wt/WSortFilterProxyModel.h>

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	class ReadChannelsFilter : public Wt::WSortFilterProxyModel
	{
		bool HideRead_ = true;
	public:
		ReadChannelsFilter ();

		void SetHideRead (bool);
	protected:
		bool filterAcceptRow (int, const Wt::WModelIndex&) const;
	};
}
}
}
