/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "avatarsstoragethread.h"

namespace LC
{
namespace Azoth
{
	QFuture<void> AvatarsStorageThread::SetAvatar (const QString& entryId,
			IHaveAvatars::Size size, const QByteArray& imageData)
	{
		return ScheduleImpl (&W::SetAvatar, entryId, size, imageData);
	}

	QFuture<std::optional<QByteArray>> AvatarsStorageThread::GetAvatar (const QString& entryId, IHaveAvatars::Size size)
	{
		return ScheduleImpl (&W::GetAvatar, entryId, size);
	}

	QFuture<void> AvatarsStorageThread::DeleteAvatars (const QString& entryId)
	{
		return ScheduleImpl (&W::DeleteAvatars, entryId);
	}
}
}
