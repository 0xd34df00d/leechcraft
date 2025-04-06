/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "poolsmanager.h"
#include "components/storage/storagebackendmanager.h"
#include "components/storage/storagebackend.h"

namespace LC::Aggregator
{
	PoolsManager& PoolsManager::Instance ()
	{
		static PoolsManager pm;
		return pm;
	}

	void PoolsManager::ReloadPools ()
	{
		decltype (Pools_) pools;

		auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		for (int type = 0; type < PTMAX; ++type)
		{
			Util::IDPool<IDType_t> pool;
			pool.SetID (sb->GetHighestID (static_cast<PoolType> (type)) + 1);
			pools [static_cast<PoolType> (type)] = pool;
		}

		Pools_ = pools;
	}

	Util::IDPool<IDType_t>& PoolsManager::GetPool (PoolType type)
	{
		return Pools_ [type];
	}

}
