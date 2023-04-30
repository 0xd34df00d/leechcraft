/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTimer>

namespace LC::Util
{
	template<typename F>
	auto Throttled (std::chrono::milliseconds ms, QObject *parent, F&& f)
	{
		return [=, scheduled = false, f = std::forward<F> (f)] () mutable
		{
			if (scheduled)
				return;

			scheduled = true;
			QTimer::singleShot (ms, Qt::VeryCoarseTimer, parent,
					[&]
					{
						std::invoke (f);
						scheduled = false;
					});
		};
	}
}
