/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC
{
namespace CpuLoad
{
	enum class LoadPriority
	{
		High,
		Medium,
		Low,
		IO,

		// This one for completeness
		Idle
	};

	struct LoadTypeInfo
	{
		double LoadPercentage_;

		LoadTypeInfo& operator-= (const LoadTypeInfo&);
	};

	LoadTypeInfo operator- (const LoadTypeInfo&,  const LoadTypeInfo&);
}
}
