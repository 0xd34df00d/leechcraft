/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <source_location>
#include "sllconfig.h"

class QMessageLogger;

namespace LC::Util
{
	[[nodiscard]]
	UTIL_SLL_API QMessageLogger LogAt (std::source_location);
}
