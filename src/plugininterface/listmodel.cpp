/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "listmodel.h"
#include <QtDebug>

namespace LeechCraft
{
	namespace Util
	{
		ListModel::ListModel (const QStringList& headers, QObject *parent)
		: QAbstractItemModel (parent)
		, Headers_ (headers)
		{
		}

		ListModel::~ListModel ()
		{
			qDeleteAll (Items_);
		}

		int ListModel::columnCount (const QModelIndex&) const
		{
			return Headers_.size ();
		}

		QVariant ListModel::data (const QModelIndex& index, int role) const
		{
			return Items_ [index.row ()]->Data (index.column (), role);
		}

		Qt::ItemFlags ListModel::flags (const QModelIndex&) const
		{
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		}

		QVariant ListModel::headerData (int section, Qt::Orientation orient, int role) const
		{
			if (orient != Qt::Horizontal ||
					role != Qt::DisplayRole)
				return QVariant ();

			return Headers_.at (section);
		}

		int ListModel::rowCount (const QModelIndex& index) const
		{
			return index.isValid () ? 0 : Items_.size ();
		}

		void ListModel::Insert (ListModelItem *item, int pos)
		{
			if (pos == -1)
				pos = Items_.size ();

			beginInsertRows (QModelIndex (), pos, pos);
			Items_.insert (pos, item);
			endInsertRows ();
		}

		void ListModel::Remove (ListModelItem *item)
		{
			int pos = Items_.indexOf (item);
			if (pos == -1)
			{
				qWarning () << Q_FUNC_INFO
					<< "not found"
					<< item;
				return;
			}

			beginRemoveRows (QModelIndex (), pos, pos);
			Items_.removeAt (pos);
			endRemoveRows ();
		}

		void ListModel::Remove (int pos)
		{
			beginRemoveRows (QModelIndex (), pos, pos);
			Items_.removeAt (pos);
			endRemoveRows ();
		}

		void ListModel::Update (ListModelItem *item)
		{
			int pos = Items_.indexOf (item);
			if (pos == -1)
			{
				qWarning () << Q_FUNC_INFO
					<< "not found"
					<< item;
				return;
			}

			Update (pos);
		}

		void ListModel::Update (int pos)
		{
			emit dataChanged (index (pos, 0),
					index (pos, columnCount () - 1));
		}

		void ListModel::SetHeaders (const QStringList& headers)
		{
			Headers_ = headers;
		}

		template<>
			QList<ListModelItem*> ListModel::GetItems () const
			{
				return Items_;
			}
	};
};

