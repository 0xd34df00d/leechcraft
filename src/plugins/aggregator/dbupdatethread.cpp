/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dbupdatethread.h"
#include <QModelIndex>
#include "common.h"
#include "dbutils.h"
#include "dbupdatethreadworker.h"

namespace LC::Aggregator
{
	QFuture<void> DBUpdateThread::SetAllChannelsRead ()
	{
		return ScheduleImpl ([] (DBUpdateThreadWorker *worker)
				{
					for (const auto& channel : GetAllChannels ())
						worker->toggleChannelUnread (channel.ChannelID_, false);
				});
	}

	QFuture<void> DBUpdateThread::ToggleChannelUnread (const QModelIndex& idx, bool unread)
	{
		const auto channelId = idx.data (ChannelRoles::ChannelID).value<IDType_t> ();
		return ScheduleImpl (&DBUpdateThreadWorker::toggleChannelUnread, channelId, unread);
	}
}
