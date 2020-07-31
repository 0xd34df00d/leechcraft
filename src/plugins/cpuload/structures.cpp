/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "structures.h"

namespace LC
{
namespace CpuLoad
{
	LoadTypeInfo& LoadTypeInfo::operator-= (const LoadTypeInfo& other)
	{
		LoadPercentage_ -= other.LoadPercentage_;
		return *this;
	}

	LoadTypeInfo operator- (const LoadTypeInfo& left, const LoadTypeInfo& right)
	{
		LoadTypeInfo res = left;
		res -= right;
		return res;
	}
}
}
