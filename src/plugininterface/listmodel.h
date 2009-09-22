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

#ifndef PLUGININTERFACE_LISTMODEL_H
#define PLUGININTERFACE_LISTMODEL_H
#include <QAbstractItemModel>
#include <QStringList>

namespace LeechCraft
{
	namespace Util
	{
		class ListModelItem
		{
		public:
			virtual ~ListModelItem () { }

			virtual QVariant Data (int, int) const = 0;
		};

		class ListModel : public QAbstractItemModel
		{
			Q_OBJECT

			QList<ListModelItem*> Items_;
			QStringList Headers_;
		public:
			enum Roles
			{
				RolePointer = Qt::UserRole + 25
			};

			ListModel (const QStringList& = QStringList (), QObject* = 0);
			virtual ~ListModel ();

			int columnCount (const QModelIndex& = QModelIndex ()) const;
			QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
			Qt::ItemFlags flags (const QModelIndex&) const;
			QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
			QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
			QModelIndex parent (const QModelIndex&) const;
			int rowCount (const QModelIndex& = QModelIndex ()) const;

			void Insert (ListModelItem*, int = -1);
			void Remove (ListModelItem*);
			void Remove (int);
			void Update (ListModelItem*);
			void Update (int);

			void SetHeaders (const QStringList&);

			template<typename T>
				QList<T*> GetItems () const
				{
					QList<T*> result;
					Q_FOREACH (ListModelItem *item, Items_)
						result << static_cast<T*> (item);
					return result;
				}
			
			template<typename T>
				T* GetItem (const QModelIndex& index) const
				{
					return GetItem<T> (index.row ());
				}

			template<typename T>
				T* GetItem (int row) const
				{
					return static_cast<T*> (Items_.at (row));
				}
		};

		template<> QList<ListModelItem*> ListModel::GetItems () const;
	};
};

#endif

