/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sllconfig.h"

class QString;

namespace LC::Util
{
	/** @brief Describes the exception currently being handled.
	 *
	 * This function shall only be called from within a `catch` block
	 * (directly or via its callees), since otherwise there is no
	 * current exception and the program terminates.
	 *
	 * @return The `what()` of an `std::exception`, or the demangled
	 * type name of any other exception.
	 */
	[[nodiscard]]
	UTIL_SLL_API QString DescribeCurrentException ();
}
