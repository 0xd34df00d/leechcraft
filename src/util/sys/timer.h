/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <string_view>
#include <QElapsedTimer>
#include "sysconfig.h"

namespace LC::Util
{
	class UTIL_SYS_API Timer final
	{
		const std::string_view Label_;

		QElapsedTimer Timer_;
	public:
		enum class Resolution
		{
			ns,
			us,
			ms
		};
	private:
		Resolution Resolution_;
	public:
		explicit Timer (std::string_view, Resolution = Resolution::ms);
		~Timer ();
	};
}
