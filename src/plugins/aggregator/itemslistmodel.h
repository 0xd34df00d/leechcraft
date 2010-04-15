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

#ifndef PLUGINS_AGGREGATOR_ITEMSLISTMODEL_H
#define PLUGINS_AGGREGATOR_ITEMSLISTMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include <QPair>
#include "item.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemsListModel : public QAbstractItemModel
			{
				Q_OBJECT

				QStringList ItemHeaders_;
				items_shorts_t CurrentItems_;
				int CurrentRow_;
				// First is ParentURL_ and second is Title_
				QPair<QString, QString> CurrentChannelHash_;
				bool MayBeRichText_;
			public:
				ItemsListModel (QObject* = 0);

				int GetSelectedRow () const;
				const QPair<QString, QString>& GetHash () const;
				void SetHash (const QPair<QString, QString>&);
				void Selected (const QModelIndex&);
				void MarkItemAsUnread (const QModelIndex&);
				const ItemShort& GetItem (const QModelIndex&) const;
				bool IsItemRead (int) const;
				QStringList GetCategories (int) const;
				void Reset (const QPair<QString, QString>&);
				void ItemDataUpdated (Item_ptr);

				int columnCount (const QModelIndex& = QModelIndex ()) const;
				QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				Qt::ItemFlags flags (const QModelIndex&) const;
				QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex& = QModelIndex ()) const;
			};
		};
	};
};

#endif

