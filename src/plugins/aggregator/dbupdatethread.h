/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QThreadPool>
#include <QFuture>
#include "channel.h"

class QModelIndex;

namespace LC::Aggregator
{
	class DBUpdateThread : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::DBUpdateThread)

		QThreadPool Pool_;
	public:
		explicit DBUpdateThread (QObject* = nullptr);

		QFuture<void> SetAllChannelsRead ();
		QFuture<void> ToggleChannelUnread (const QModelIndex&, bool unread);

		QFuture<void> UpdateFeed (channels_container_t channels, QString url);
	};

	using DBUpdateThread_ptr = std::shared_ptr<DBUpdateThread>;
}
