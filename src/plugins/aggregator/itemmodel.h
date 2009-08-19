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

#ifndef PLUGINS_AGGREGATOR_ITEMMODEL_H
#define PLUGINS_AGGREGATOR_ITEMMODEL_H
#include <deque>
#include <QAbstractItemModel>
#include <QStringList>
#include "item.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemModel : public QAbstractItemModel
			{
				Q_OBJECT

				std::deque<Item_ptr> Items_;
				QStringList ItemHeaders_;
				bool SaveScheduled_;
			public:
				ItemModel (QObject* = 0);
				virtual ~ItemModel ();

				void AddItem (const Item_ptr&);
				void RemoveItem (const QModelIndex&);
				void Activated (const QModelIndex&) const;
				QString GetDescription (const QModelIndex&) const;

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual bool hasChildren (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			private:
				void ScheduleSave ();
			public slots:
				void saveSettings ();
			};
		};
	};
};

#endif

