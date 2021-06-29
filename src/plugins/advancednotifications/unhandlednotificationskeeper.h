/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QCoreApplication>

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
		Q_DECLARE_TR_FUNCTIONS (LC::AdvancedNotifications::UnhandledNotificationsKeeper)

		QStandardItemModel * const Model_;
	public:
		explicit UnhandledNotificationsKeeper (QObject* = nullptr);

		void AddUnhandled (const Entity&);

		QAbstractItemModel* GetUnhandledModel () const;

		QList<Entity> GetRulesEntities (const QList<QModelIndex>&) const;
	};
}
}
