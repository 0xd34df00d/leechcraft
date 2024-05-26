/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "guiconfig.h"
#include <QList>

class QAbstractItemModel;
class QAction;
class QMenu;

namespace LC::Util
{
	struct MenuModelOptions
	{
		QList<QAction*> AdditionalActions_ {};
	};

	UTIL_GUI_API void SetMenuModel (QMenu& menu,
			QAbstractItemModel& model,
			MenuModelOptions options = {});
}
