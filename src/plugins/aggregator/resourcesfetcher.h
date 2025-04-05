/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/eitherfwd.h>
#include <util/sll/void.h>
#include <util/threads/coro/taskfwd.h>
#include "common.h"

class QString;

namespace LC::Aggregator
{
	struct Channel;

	Util::Task<Util::Either<QString, Util::Void>> UpdateFavicon (IDType_t channelId, const QString& channelLink);
	Util::Task<Util::Either<QString, Util::Void>> UpdatePixmap (IDType_t channelId, const QString& pixmapLink);

	void UpdateChannelResources (const Channel&);
}
