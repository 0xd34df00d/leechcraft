/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "convinfo.h"

namespace LC
{
namespace GmailNotifier
{
	bool operator== (const ConvInfo& i1, const ConvInfo& i2)
	{
		return i1.Link_ == i2.Link_;
	}
}
}
