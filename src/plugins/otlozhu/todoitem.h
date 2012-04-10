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

#include <memory>
#include <QStringList>
#include <QDateTime>

namespace LeechCraft
{
namespace Otlozhu
{
	class TodoItem;
	typedef std::shared_ptr<TodoItem> TodoItem_ptr;

	class TodoItem
	{
		QString ID_;

		QString Title_;
		QString Comment_;
		QStringList TagIDs_;
		QDateTime Created_;
		QDateTime Due_;

		int Percentage_;
		QStringList Deps_;
	public:
		TodoItem ();
		TodoItem (const QString&);
		TodoItem (const TodoItem&) = delete;
		TodoItem& operator= (const TodoItem&) = delete;

		TodoItem_ptr Clone () const;
		void CopyFrom (const TodoItem_ptr);

		static TodoItem_ptr Deserialize (const QByteArray&);
		QByteArray Serialize () const;

		QString GetID () const;

		QString GetTitle () const;
		void SetTitle (const QString&);

		QString GetComment () const;
		void SetComment (const QString&);

		QStringList GetTagIDs () const;
		void SetTagIDs (const QStringList&);

		QDateTime GetCreatedDate () const;
		void SetCreatedDate (const QDateTime&);

		QDateTime GetDueDate () const;
		void SetDueDate (const QDateTime&);

		int GetPercentage () const;
		void SetPercentage (int);

		QStringList GetDeps () const;
		void SetDeps (const QStringList&);
		void AddDep (const QString&);
	};
}
}
