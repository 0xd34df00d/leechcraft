/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QCoreApplication>
#include <QObject>
#include <util/threads/coro/taskfwd.h>
#include <util/threads/coro/throttle.h>
#include "common.h"
#include "dbupdatethread.h"

class QTimer;

namespace LC::Aggregator
{
	class FeedsErrorManager;

	class UpdatesManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::UpdatesManager)

		const DBUpdateThread_ptr DBUpThread_;
		const std::shared_ptr<FeedsErrorManager> FeedsErrorManager_;

		QTimer * const UpdateTimer_;
		QTimer * const CustomUpdateTimer_;

		QMap<IDType_t, QDateTime> Updates_;

		Util::Throttle UpdateThrottle_;
	public:
		struct InitParams
		{
			const DBUpdateThread_ptr DBUpThread_;
			const std::shared_ptr<FeedsErrorManager>& FeedsErrorManager_;
		};
		explicit UpdatesManager (const InitParams&, QObject* = nullptr);

		void UpdateFeed (IDType_t);
		void UpdateFeeds ();
	private:
		void HandleCustomUpdates ();

		Util::ContextTask<void> UpdateFeedAsync (IDType_t);
	};
}
