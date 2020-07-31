/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxyfwd.h>

class QAbstractItemModel;
class QStandardItemModel;
class QModelIndex;

namespace LC
{
struct Entity;

namespace AdvancedNotifications
{
	class NotificationRule;

	class UnhandledNotificationsKeeper : public QObject
	{
		const ICoreProxy_ptr Proxy_;
		QStandardItemModel * const Model_;
	public:
		UnhandledNotificationsKeeper (const ICoreProxy_ptr&, QObject* = nullptr);

		void AddUnhandled (const Entity&);

		QAbstractItemModel* GetUnhandledModel () const;

		QList<Entity> GetRulesEntities (const QList<QModelIndex>&) const;
	};
}
}
