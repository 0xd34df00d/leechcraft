/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <variant>
#include "guiconfig.h"

class QWidget;
class QFont;

namespace LC::Util
{
	template<typename T>
	struct FontSizeChangerMethods
	{
		std::function<T ()> GetView_;
		std::function<void (T)> SetView_;
		std::function<void (T)> SetDefault_;
	};

	using PixelBasedParams = FontSizeChangerMethods<int>;
	using FontBasedParams = FontSizeChangerMethods<QFont>;

	using FontSizeChangerParams = std::variant<PixelBasedParams, FontBasedParams>;

	UTIL_GUI_API void InstallFontSizeChanger (QWidget&, const FontSizeChangerParams&);
}
