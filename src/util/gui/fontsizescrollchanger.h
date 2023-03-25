/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include "guiconfig.h"

class QWidget;

namespace LC::Util
{
	struct FontSizeScrollChangerMixingParams
	{
		std::function<int ()> GetViewFontSize_;
		std::function<void (int)> SetViewFontSize_;
		std::function<void (int)> SetDefaultFontSize_;
	};

	UTIL_GUI_API void InstallFontSizeScrollChanger (QWidget&, const FontSizeScrollChangerMixingParams&);
}
