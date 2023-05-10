/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "common.h"
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Aggregator
{
	const QString MessageBoxTitle { "LeechCraft Aggregator"_qs };

	int ToRowDelta (ChannelDirection dir)
	{
		switch (dir)
		{
		case ChannelDirection::PreviousUnread:
			return -1;
		case ChannelDirection::NextUnread:
			return +1;
		}

		qWarning () << "ToRowDelta: unknown direction"
				<< static_cast<int> (dir);
		return -1;
	}
}
