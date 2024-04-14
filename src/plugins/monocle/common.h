/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QString;


namespace LC::Monocle
{
	enum class LayoutMode
	{
		OnePage,
		TwoPages,
		TwoPagesShifted
	};

	enum class ScaleMode
	{
		Fixed,
		FitWidth,
		FitPage
	};

	QString LayoutMode2Name (LayoutMode mode);
	LayoutMode Name2LayoutMode (const QString&);
}
