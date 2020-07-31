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
namespace Azoth
{
namespace Murm
{
	bool operator== (const AppInfo& left, const AppInfo& right)
	{
		return left.AppId_ == right.AppId_ &&
				left.IsMobile_ == right.IsMobile_ &&
				left.Title_ == right.Title_ &&
				left.Icon25_ == right.Icon25_;
	}

	UserInfo UserInfo::FromID (qulonglong id)
	{
		UserInfo info {};
		info.ID_ = id;
		return info;
	}
}
}
}
