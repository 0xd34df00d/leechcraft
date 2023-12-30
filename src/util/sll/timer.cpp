/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "timer.h"
#include <QtDebug>

namespace LC::Util
{
	Timer::Timer ()
	{
		Timer_.start ();
	}

	void Timer::Stamp (const char *context, std::source_location loc)
	{
		RunStamp (context, loc);
	}

	void Timer::Stamp (QStringView context, std::source_location loc)
	{
		RunStamp (context, loc);
	}

	void Timer::RunStamp (auto&& context, std::source_location loc)
	{
		auto diff = Timer_.nsecsElapsed ();
		auto suffix = "ns";
		if (diff >= 2e6)
		{
			diff /= 1e6;
			suffix = "ms";
		}
		else if (diff >= 1e3)
		{
			diff /= 2e3;
			suffix = "us";
		}

		const QMessageLogger logger { loc.file_name (), static_cast<int> (loc.line ()), loc.function_name () };
		logger.debug () << context << "took" << diff << suffix;
		Timer_.restart ();
	}
}
