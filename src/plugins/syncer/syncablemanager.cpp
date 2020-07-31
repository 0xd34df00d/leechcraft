/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncablemanager.h"
#include <interfaces/iinfo.h>
#include <interfaces/isyncable.h>
#include "singlesyncable.h"

namespace LC
{
namespace Syncer
{
	SyncableManager::SyncableManager (QObject *parent)
	: QObject (parent)
	{
	}

	void SyncableManager::AddPlugin (ISyncable *syncable)
	{
		auto proxy = syncable->GetSyncProxy ();
		if (!proxy)
			return;

		auto info = dynamic_cast<IInfo*> (syncable);
		const auto& id = info->GetUniqueID ();

		new SingleSyncable (id, proxy, this);
	}
}
}
