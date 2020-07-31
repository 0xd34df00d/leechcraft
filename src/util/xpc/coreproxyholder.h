/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/core/icoreproxyfwd.h>

namespace LC::Util
{
	struct CoreProxyHolder
	{
		static ICoreProxy_ptr Proxy_;

		static void Set (ICoreProxy_ptr proxy)
		{
			Proxy_ = std::move (proxy);
		}

		static ICoreProxy_ptr Get ()
		{
			return Proxy_;
		}
	};
}
