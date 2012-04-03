/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSettings>
#include "todoitem.h"

namespace LeechCraft
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
		void HandleUpdated (TodoItem_ptr);
		void RemoveItem (const QString&);
	private:
		void Load ();
		void SaveAt (int);
		void SaveAt (const QList<int>&);
	signals:
		void itemAdded (int);
		void itemRemoved (int);
		void itemUpdated (int);
	};
}
}
