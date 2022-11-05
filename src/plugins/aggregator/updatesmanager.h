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
#include "common.h"
#include "dbupdatethread.h"

class QTimer;

class IEntityManager;

namespace LC::Aggregator
{
	class FeedsErrorManager;

	class UpdatesManager : public QObject
	{
		Q_OBJECT

		IEntityManager * const EntityManager_;

		const DBUpdateThread_ptr DBUpThread_;
		const std::shared_ptr<FeedsErrorManager> FeedsErrorManager_;

		QTimer * const UpdateTimer_;
		QTimer * const CustomUpdateTimer_;

		QList<IDType_t> UpdatesQueue_;
		QMap<IDType_t, QDateTime> Updates_;
	public:
		struct InitParams
		{
			const DBUpdateThread_ptr DBUpThread_;
			const std::shared_ptr<FeedsErrorManager>& FeedsErrorManager_;
			IEntityManager *EntityManager_;
		};
		explicit UpdatesManager (const InitParams&, QObject* = nullptr);

		void UpdateFeed (IDType_t);

		void UpdateFeeds ();
	private:
		void HandleCustomUpdates ();
		void RotateUpdatesQueue ();
	private slots:
		void updateIntervalChanged ();
	};
}
