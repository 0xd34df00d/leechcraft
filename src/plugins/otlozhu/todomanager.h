/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QAbstractItemModel;

namespace LC
{
struct Entity;

namespace Otlozhu
{
	class TodoStorage;
	class StorageModel;
	class NotificationsManager;

	class TodoManager : public QObject
	{
		Q_OBJECT

		const QString Context_;
		TodoStorage *Storage_;
		StorageModel *Model_;
		NotificationsManager *NotifMgr_;
	public:
		TodoManager (const QString&, QObject* = 0);

		TodoStorage* GetTodoStorage () const;
		QAbstractItemModel* GetTodoModel () const;
	signals:
		void gotEntity (const LC::Entity&);
	};
}
}
