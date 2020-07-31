/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fdguard.h"

#include <fcntl.h>
#include <unistd.h>

namespace LC
{
namespace Util
{
	FDGuard::FDGuard (const char *file, int mode)
	: FD_ { open (file, mode) }
	{
	}

	FDGuard::FDGuard (FDGuard&& other)
	: FD_ { other.FD_ }
	{
		other.FD_ = -1;
	}

	FDGuard& FDGuard::operator= (FDGuard&& other)
	{
		swap (*this, other);

		return *this;
	}

	FDGuard::~FDGuard ()
	{
		if (FD_ >= 0)
			close (FD_);
	}

	FDGuard::operator bool () const
	{
		return FD_ >= 0;
	}

	FDGuard::operator int () const
	{
		return FD_;
	}

	void swap (FDGuard& g1, FDGuard& g2)
	{
		std::swap (g1.FD_, g2.FD_);
	}
}
}
