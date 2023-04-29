/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <string>
#include <QVector>
#include "guiconfig.h"

class QSplitter;

namespace LC::Util
{
	class BaseSettingsManager;

	struct StateSaverParams
	{
		BaseSettingsManager& XSM_;
		std::string Id_;
		QVector<std::optional<int>> InitialWidths_;
	};

	UTIL_GUI_API void SetupStateSaver (QSplitter&, const StateSaverParams&);
}
