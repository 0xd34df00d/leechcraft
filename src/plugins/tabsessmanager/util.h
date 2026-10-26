/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QIcon;

struct TabClassInfo;

class ITabWidget;

namespace LC
{
	struct TabSaveInfo;
}

namespace LC::TabSessManager
{
	bool IsGoodSingleTC (const TabClassInfo&);

	QIcon GetTabIcon (ITabWidget& tab, const TabSaveInfo& info);
}
