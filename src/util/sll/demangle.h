/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <typeinfo>
#include "sllconfig.h"

class QString;

namespace LC::Util
{
	/** @brief Returns a human-readable form of a mangled symbol name.
	 *
	 * @param[in] name The possibly-mangled name, e.g. as returned by
	 * std::type_info::name().
	 * @return The demangled, human-readable name.
	 *
	 * @sa Demangle(const std::type_info&)
	 */
	UTIL_SLL_API QString Demangle (const char *name);

	/** @brief Convenience overload demangling a type_info's name.
	 *
	 * Equivalent to `Demangle(info.name ())`, convenient for
	 * `Demangle(typeid(x))`.
	 *
	 * @param[in] info The type info whose name shall be demangled.
	 * @return The demangled, human-readable name.
	 *
	 * @sa Demangle(const char*)
	 */
	UTIL_SLL_API QString Demangle (const std::type_info& info);
}
