/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <tox/tox.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class ToxLogger
	{
		const QString Name_;
	public:
		ToxLogger (const QString&);

		void Log (TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func, const char *message);
	};
}
}
}
