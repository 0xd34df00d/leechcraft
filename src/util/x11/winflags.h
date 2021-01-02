/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QFlags>

namespace LC::Util
{
	enum WinStateFlag
	{
		NoState			= 0,
		Modal			= 1 << 0,
		Sticky			= 1 << 1,
		MaximizedVert	= 1 << 2,
		MaximizedHorz	= 1 << 3,
		Shaded			= 1 << 4,
		SkipTaskbar		= 1 << 5,
		SkipPager		= 1 << 6,
		Hidden			= 1 << 7,
		Fullscreen		= 1 << 8,
		OnTop			= 1 << 9,
		OnBottom		= 1 << 10,
		Attention		= 1 << 11
	};

	Q_DECLARE_FLAGS (WinStateFlags, WinStateFlag)

	enum AllowedActionFlag
	{
		NoAction		= 0,
		Move			= 1 << 0,
		Resize			= 1 << 1,
		Minimize		= 1 << 2,
		Shade			= 1 << 3,
		Stick			= 1 << 4,
		MaximizeHorz	= 1 << 5,
		MaximizeVert	= 1 << 6,
		ShowFullscreen	= 1 << 7,
		ChangeDesktop	= 1 << 8,
		Close			= 1 << 9,
		MoveToTop		= 1 << 10,
		MoveToBottom	= 1 << 11
	};

	Q_DECLARE_FLAGS (AllowedActionFlags, AllowedActionFlag)
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Util::WinStateFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Util::AllowedActionFlags)
