/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstdint>
#include <variant>

class QAction;
class QString;
class QToolBar;
class QWidget;

template<typename>
class QVector;

namespace LC::Monocle
{
	enum class LayoutMode : std::uint8_t
	{
		OnePage,
		TwoPages,
		TwoPagesShifted
	};

	struct FitWidth {};
	struct FitPage {};
	struct FixedScale
	{
		double Scale_ = 1;
	};

	using ScaleMode = std::variant<FitWidth, FitPage, FixedScale>;

	QString LayoutMode2Name (LayoutMode mode);
	LayoutMode Name2LayoutMode (const QString&);

	using ToolbarEntry = std::variant<QWidget*, QAction*>;

	void AddToolbarEntries (QToolBar&, const QVector<ToolbarEntry>&);
}
