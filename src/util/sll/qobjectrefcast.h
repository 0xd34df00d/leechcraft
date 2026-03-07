/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <concepts>
#include <source_location>
#include <stdexcept>
#include <QObject>
#include "sllconfig.h"

namespace LC::Util
{
	struct BadQObjectCast : std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	namespace detail
	{
		[[noreturn, gnu::cold, gnu::noinline]]
		void UTIL_SLL_API NotifyCastError (const QObject *object, const char *target, const std::source_location& loc);
	}
}

namespace LC
{
	template<typename Target, std::derived_from<QObject> Src>
		requires (!std::is_pointer_v<Target>)
	decltype (auto) qobject_ref_cast (Src *obj, std::source_location loc = std::source_location::current ())
	{
		if (const auto result = qobject_cast<Target*> (obj))
			return *result;

		Util::detail::NotifyCastError (obj, typeid (Target).name (), loc);
	}

	template<typename Target, std::derived_from<QObject> Src>
		requires (!std::is_pointer_v<Target>)
	decltype (auto) qobject_ref_cast (Src& obj, std::source_location loc = std::source_location::current ())
	{
		return qobject_ref_cast<Target> (&obj, loc);
	}
}
