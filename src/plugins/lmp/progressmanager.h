/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/ijobholder.h>

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LC::LMP
{
	class SyncManagerBase;

	class ProgressManager : public QObject
	{
		QStandardItemModel *Model_;
	public:
		explicit ProgressManager (QObject* = nullptr);

		QAbstractItemModel* GetModel () const;

		class Handle final
		{
			friend class ProgressManager;

			QList<QStandardItem*> Row_;
			QString StatusPattern_;
			int Done_ = 0;
			int Total_ = 0;

			explicit Handle (QList<QStandardItem*> row, const QString& statusPattern, std::optional<int> total);
		public:
			~Handle ();

			Handle (const Handle&) = delete;
			Handle& operator= (const Handle&) = delete;

			Handle (Handle&&) = default;
			Handle& operator= (Handle&&) = default;

			void Update (int done, int total);
			void Update (int done);

			void operator++ ();
		private:
			void Update () const;
		};

		struct Item
		{
			QString Name_;
			QString StatusPattern_;
			JobHolderRow Type_;
			std::optional<int> Total_;
		};

		Handle Add (const Item&);
	};
}
