/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Andrey Batyiev
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

#include "objectlistmodel.h"

namespace LeechCraft
{
namespace Util
{
	ObjectListModel::ObjectListModel (QObject *parent)
	: QAbstractListModel (parent)
	{
		QHash<int, QByteArray> roles;
		roles [DataRoles::ObjectRole] = "object";
		setRoleNames (roles);
	}

	QVariant ObjectListModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		if (role == DataRoles::ObjectRole)
			return QVariant::fromValue (Items_ [index.row ()]);

		return QVariant ();
	}

	int ObjectListModel::rowCount (const QModelIndex& parent) const
	{
		return Items_.size ();
	}

	void ObjectListModel::Append (QObject *object)
	{
		beginInsertRows (QModelIndex (), Items_.count (), Items_.count ());
		Items_.append (object);
		endInsertRows ();
	}

	void ObjectListModel::Prepend (QObject *object)
	{
		beginInsertRows (QModelIndex (), 0, 0);
		Items_.prepend (object);
		endInsertRows ();
	}

	void ObjectListModel::Remove (QObject *object)
	{
		int position = 0;
		Q_FOREACH (QObject *item, Items_)
		{
			if (item == object)
				Remove (position);
			else
				position++;
		}
	}

	void ObjectListModel::Remove (int position)
	{
		beginRemoveRows (QModelIndex (), position, position);
		Items_.removeAt (position);
		endRemoveRows ();
	}

	void ObjectListModel::SetObjects (const QList<QObject*>& data)
	{
		beginResetModel ();
		Items_ = data;
		endResetModel ();
		emit dataChanged (index (0), index (Items_.count () - 1));
	}

	QList<QObject*> ObjectListModel::GetObjects () const
	{
		return Items_;
	}
}
}