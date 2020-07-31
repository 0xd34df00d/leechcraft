/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QSettings>
#include "todoitem.h"

namespace LC
{
namespace Otlozhu
{
	class TodoStorage : public QObject
	{
		Q_OBJECT

		const QString Context_;
		QList<TodoItem_ptr> Items_;

		QSettings Storage_;
	public:
		TodoStorage (const QString&, QObject* = 0);

		int GetNumItems () const;
		int FindItem (const QString&) const;

		void AddItem (TodoItem_ptr);
		TodoItem_ptr GetItemAt (int idx) const;
		TodoItem_ptr GetItemByID (const QString&) const;
		QList<TodoItem_ptr> GetAllItems () const;

		void AddDependency (const QString& itemId, const QString& depId);

		void HandleUpdated (TodoItem_ptr);
		void RemoveItem (const QString&);
	private:
		void HandleUpdated (TodoItem_ptr, const std::function<void ()>&);
		void Load ();
		void SaveAt (int);
		void SaveAt (const QList<int>&);
	signals:
		void itemAdded (int);
		void itemRemoved (int);
		void itemUpdated (int);
		void itemDiffGenerated (const QString&, const QVariantMap&);

		void itemDepAdded (int itemIdx, int depIdx);
		void itemDepRemoved (int itemIdx, int depIdx);
	};
}
}
