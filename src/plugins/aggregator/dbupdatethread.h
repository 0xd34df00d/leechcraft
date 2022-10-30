/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/threads/workerthreadbase.h>

class QModelIndex;

namespace LC::Aggregator
{
	class DBUpdateThreadWorker;

	class DBUpdateThread : public Util::WorkerThread<DBUpdateThreadWorker>
	{
	public:
		using WorkerThread::WorkerThread;

		QFuture<void> SetAllChannelsRead ();

		QFuture<void> ToggleChannelUnread (const QModelIndex&, bool unread);
	};

	using DBUpdateThread_ptr = std::shared_ptr<DBUpdateThread>;
}
