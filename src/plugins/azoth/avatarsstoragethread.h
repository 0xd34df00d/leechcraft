/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <util/threads/workerthreadbase.h>
#include "interfaces/azoth/ihaveavatars.h"
#include "avatarsstorageondisk.h"

namespace LC
{
namespace Azoth
{
	class AvatarsStorageOnDisk;

	class AvatarsStorageThread final : public Util::WorkerThread<AvatarsStorageOnDisk>
	{
	public:
		using WorkerThread::WorkerThread;

		QFuture<void> SetAvatar (const QString& entryId, IHaveAvatars::Size size, const QByteArray& imageData);
		QFuture<std::optional<QByteArray>> GetAvatar (const QString& entryId, IHaveAvatars::Size size);
		QFuture<void> DeleteAvatars (const QString& entryId);
	};
}
}
