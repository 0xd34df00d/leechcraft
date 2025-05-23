/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "feed.h"

namespace LC::Aggregator::OPML
{
	QByteArray Write (const channels_shorts_t&,
			const QString&,
			const QString&,
			const QString&);
}
