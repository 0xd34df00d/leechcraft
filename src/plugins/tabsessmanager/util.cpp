/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>

namespace LC::TabSessManager
{
	bool IsGoodSingleTC (const TabClassInfo& tc)
	{
		return tc.Features_ & TabFeature::TFSingle && tc.Features_ & TabFeature::TFOpenableByRequest;
	}

	QIcon GetTabIcon (ITabWidget& tab, const TabSaveInfo& info)
	{
		return info.Icon_.isNull () ? tab.GetTabClassInfo ().Icon_ : info.Icon_;
	}
}
