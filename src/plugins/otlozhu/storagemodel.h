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

#include <QAbstractItemModel>
#include <QStringList>

namespace LeechCraft
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
	private slots:
		void handleItemAdded (int);
		void handleItemUpdated (int);
		void handleItemRemoved (int);
	};
}
}
