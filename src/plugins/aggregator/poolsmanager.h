/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <util/idpool.h>
#include "common.h"

namespace LC::Aggregator
{
	class PoolsManager
	{
		QHash<PoolType, Util::IDPool<IDType_t>> Pools_;
	public:
		static PoolsManager& Instance ();

		void ReloadPools ();

		Util::IDPool<IDType_t>& GetPool (PoolType);
	};
}
