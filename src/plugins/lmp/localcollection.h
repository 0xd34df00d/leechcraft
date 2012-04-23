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
#include <QHash>
#include <QSet>
#include "collectiontypes.h"

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;
class QModelIndex;

namespace LeechCraft
{
namespace LMP
{
	class LocalCollectionStorage;
	class Player;

	class LocalCollection : public QObject
	{
		Q_OBJECT

		LocalCollectionStorage *Storage_;
		QStandardItemModel *CollectionModel_;

		Collection::Artists_t Artists_;
		QSet<QString> PresentPaths_;

		QHash<int, QStandardItem*> Artist2Item_;
		QHash<int, QStandardItem*> Album2Item_;
	public:
		LocalCollection (QObject* = 0);

		QAbstractItemModel* GetCollectionModel () const;

		void Scan (const QString&);
	private:
		void AppendToModel (const Collection::Artists_t&);
	};
}
}
