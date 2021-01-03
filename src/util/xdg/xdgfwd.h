/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "xdgconfig.h"

namespace LC::Util::XDG
{
	class UTIL_XDG_API Item;
	using Item_ptr = std::shared_ptr<Item>;

	class UTIL_XDG_API ItemsFinder;
}
