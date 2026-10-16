/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "exceptions.h"
#include <exception>
#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#define LC_HAS_CXA_CURRENT_EXCEPTION_TYPE
#endif
#include <QString>
#include "demangle.h"
#include "qtutil.h"

namespace LC::Util
{
	QString DescribeCurrentException ()
	{
		try
		{
			throw;
		}
		catch (const std::exception& e)
		{
			return QString::fromUtf8 (e.what ());
		}
		catch (...)
		{
#ifdef LC_HAS_CXA_CURRENT_EXCEPTION_TYPE
			if (const auto type = abi::__cxa_current_exception_type ())
				return "exception of type "_qs + Demangle (*type);
#endif
			return "unknown exception"_qs;
		}
	}
}
