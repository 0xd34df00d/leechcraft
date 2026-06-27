/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "demangle.h"
#if __has_include(<cxxabi.h>)
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#define LC_HAS_CXA_DEMANGLE
#endif
#include <QString>

namespace LC::Util
{
	QString Demangle (const char *name)
	{
#ifdef LC_HAS_CXA_DEMANGLE
		const std::unique_ptr<char, decltype (&std::free)> demangled
		{
			abi::__cxa_demangle (name, nullptr, nullptr, nullptr),
			&std::free
		};
		return demangled ? QString::fromLatin1 (demangled.get ()) : QString::fromLatin1 (name);
#else
		return QString::fromLatin1 (name);
#endif
	}

	QString Demangle (const std::type_info& info)
	{
		return Demangle (info.name ());
	}
}
