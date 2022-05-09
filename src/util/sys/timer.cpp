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
	Timer::Timer (std::string_view label, Resolution resolution)
	: Label_ { label }
	, Resolution_ { resolution }
	{
		Timer_.start ();
	}

	Timer::~Timer ()
	{
		auto diff = Timer_.nsecsElapsed ();

		const char *unit = nullptr;
		switch (Resolution_)
		{
		case Resolution::ns:
			unit = "ns";
			break;
		case Resolution::us:
			diff /= 1e3;
			unit = "us";
			break;
		case Resolution::ms:
			diff /= 1e6;
			unit = "ms";
			break;
		}

		qDebug () << Label_.data ()    // TODO Qt6: no need for data() here
				<< "took"
				<< diff
				<< unit;
	}
}
