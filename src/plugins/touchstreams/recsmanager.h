/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <optional>
#include <QObject>
#include <QPair>
#include <util/sll/util.h>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;

namespace LC
{
namespace Util
{
namespace SvcAuth
{
	class VkAuthManager;
}

class QueueManager;
enum class QueuePriority;
}

namespace TouchStreams
{
	class RecsManager : public QObject
	{
		Q_OBJECT

		const std::optional<qlonglong> UID_;
		Util::SvcAuth::VkAuthManager * const AuthMgr_;
		Util::QueueManager * const QueueMgr_;
		const Util::DefaultScopeGuard RequestQueueGuard_;

		const ICoreProxy_ptr Proxy_;

		QList<QPair<std::function<void (QString)>, Util::QueuePriority>> RequestQueue_;

		QStandardItem * const RootItem_;
	public:
		RecsManager (std::optional<qulonglong>,
				Util::SvcAuth::VkAuthManager*,
				Util::QueueManager*,
				const ICoreProxy_ptr&,
				QObject* = 0);

		QStandardItem* GetRootItem () const;

		void RefreshItems (QList<QStandardItem*>&);
	private slots:
		void refetchRecs ();
		void handleRecsFetched ();
	};
}
}
