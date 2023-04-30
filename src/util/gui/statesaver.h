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
#include <variant>
#include <QVector>
#include "guiconfig.h"

class QSplitter;

namespace LC::Util
{
	class BaseSettingsManager;

	struct Widths : QVector<std::optional<int>>
	{
		using QVector::QVector;
	};

	struct Factors : QVector<int>
	{
		using QVector::QVector;
	};

	using InitialDistr = std::variant<Widths, Factors>;

	struct StateSaverParams
	{
		BaseSettingsManager& XSM_;
		std::string Id_;
		InitialDistr Initial_;
	};

	UTIL_GUI_API void SetupStateSaver (QSplitter&, const StateSaverParams&);
}
