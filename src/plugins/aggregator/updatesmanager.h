/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <util/threads/coro/taskfwd.h>
#include <util/threads/coro/throttle.h>
#include <util/threads/coro/corocontext.h>
#include "common.h"
#include "dbupdatethread.h"

class QTimer;

namespace LC::Util
{
	class ProgressManager;
	class ProgressModelRow;
}

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

		QHash<IDType_t, QDateTime> Updates_;

		Util::Throttle UpdateThrottle_;

		Util::ProgressManager& ProgressManager_;
	public:
		struct InitParams
		{
			const DBUpdateThread_ptr DBUpThread_;
			const std::shared_ptr<FeedsErrorManager>& FeedsErrorManager_;
		};
		explicit UpdatesManager (const InitParams&, QObject* = nullptr);

		IJobHolderRepresentationHandler_ptr CreateJobRepresentationHandler ();

		void UpdateFeed (IDType_t);
		void UpdateFeeds ();
	private:
		void RestartUpdateTimer (std::chrono::minutes);
		void HandleCustomUpdates ();

		Util::ContextTask<void> UpdateFeedsAsync (ids_t, SQLStorageBackend_ptr);
		Util::ContextTask<void> UpdateFeedAsync (IDType_t, Util::ProgressModelRow&, SQLStorageBackend_ptr);

		LC_CORO_CONTEXT
	};
}
