/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <tuple>
#include <QObject>
#include <QDBusInterface>
#include <QPointer>
#include <QStringList>
#include <interfaces/structures.h>
#include <util/threads/coro/taskfwd.h>
#include <util/threads/coro/corocontext.h>

namespace LC
{
namespace Sysnotify
{
	class NotificationManager : public QObject
	{
		Q_OBJECT

		std::unique_ptr<QDBusInterface> Connection_;

		struct ActionData
		{
			Entity E_;
			QObject_ptr Handler_;
			QStringList Actions_;
		};
		QMap<uint, ActionData> CallID2AD_;

		std::tuple<int, int> Version_ { 0, 0 };
	public:
		NotificationManager (QObject* = 0);

		bool CouldNotify (const Entity&) const;
		Util::ContextTask<void> HandleNotification (Entity);
	private:
		Util::ContextTask<void> CheckServer ();
	private slots:
		void handleActionInvoked (uint, QString);
		void handleNotificationClosed (uint);

		LC_CORO_CONTEXT
	};
}
}
