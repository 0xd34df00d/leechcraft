/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sysconfig.h"

namespace LC::Util
{
	class UTIL_SYS_API FDGuard
	{
		int FD_;
	public:
		FDGuard (const char *file, int mode);
		FDGuard (const FDGuard&) = delete;
		FDGuard (FDGuard&& other);
		~FDGuard ();

		FDGuard& operator= (const FDGuard&) = delete;
		FDGuard& operator= (FDGuard&& other);

		explicit operator bool () const;
		explicit (false) operator int () const;

		friend void swap (FDGuard& g1, FDGuard& g2);
	};
}
