/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recinfo.h"

namespace LC
{
namespace TabSessManager
{
	bool operator== (const RecInfo& r1, const RecInfo& r2)
	{
		return r1.Name_ == r2.Name_ &&
				r1.Data_ == r2.Data_ &&
				r1.WindowID_ == r2.WindowID_;
	}
}
}
