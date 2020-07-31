/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>

namespace LC
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
		QDateTime Created_ = QDateTime::currentDateTime ();
		QDateTime Due_;

		int Percentage_ = 0;
		QStringList Deps_;
	public:
		TodoItem ();
		explicit TodoItem (const QString&);
		TodoItem (const TodoItem&) = delete;
		TodoItem& operator= (const TodoItem&) = delete;

		TodoItem_ptr Clone () const;
		void CopyFrom (const TodoItem_ptr);

		QVariantMap ToMap () const;
		QVariantMap DiffWith (const TodoItem_ptr) const;
		void ApplyDiff (const QVariantMap&);

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
