/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include "todoitem.h"

namespace LC
{
namespace Otlozhu
{
	class TodoStorage;

	class StorageModel : public QAbstractItemModel
	{
		Q_OBJECT

		TodoStorage *Storage_;
		QStringList Headers_;
	public:
		enum Columns
		{
			Title,
			Tags,
			DueDate,
			Created,
			Percentage,
			MAX
		};

		enum Roles
		{
			ItemID = Qt::UserRole + 1,
			ItemTitle,
			ItemTags,
			ItemProgress,
			ItemComment,
			ItemDueDate
		};

		StorageModel (QObject* = 0);

		QVariant headerData (int section, Qt::Orientation orientation, int role) const;
		QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex& child) const;
		int rowCount (const QModelIndex& parent) const;
		int columnCount (const QModelIndex& parent) const;
		Qt::ItemFlags flags (const QModelIndex& index) const;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
		bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

		void SetStorage (TodoStorage*);
	private:
		TodoItem_ptr GetItemForIndex (const QModelIndex&) const;
	private slots:
		void handleItemAdded (int);
		void handleItemUpdated (int);
		void handleItemRemoved (int);

		void handleItemDepAdded (int, int);
		void handleItemDepRemoved (int, int);
	};
}
}
