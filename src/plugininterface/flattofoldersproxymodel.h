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

#ifndef PLUGININTERFACE_FLATTOFOLDERSPROXYMODEL_H
#define PLUGININTERFACE_FLATTOFOLDERSPROXYMODEL_H
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include <QStringList>
#include <QMultiHash>
#include "config.h"

class ITagsManager;

namespace LeechCraft
{
	struct FlatTreeItem;
	typedef boost::shared_ptr<FlatTreeItem> FlatTreeItem_ptr;

	namespace Util
	{
		class PLUGININTERFACE_API FlatToFoldersProxyModel : public QAbstractItemModel
		{
			Q_OBJECT

			QAbstractItemModel *SourceModel_;

			ITagsManager *TM_;

			FlatTreeItem_ptr Root_;
			QMultiHash<QPersistentModelIndex, FlatTreeItem_ptr> Items_;
		public:
			FlatToFoldersProxyModel (QObject* = 0);

			void SetTagsManager (ITagsManager*);

			virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
			virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
			virtual QVariant headerData (int, Qt::Orientation, int) const;
			virtual Qt::ItemFlags flags (const QModelIndex&) const;
			virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
			virtual QModelIndex parent (const QModelIndex&) const;
			virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

			void SetSourceModel (QAbstractItemModel*);
			QModelIndex MapToSource (const QModelIndex&) const;
		private:
			void HandleRowInserted (int);
			void HandleRowRemoved (int);
			void AddForTag (const QString&, const QPersistentModelIndex&);
			void RemoveFromTag (const QString&, const QPersistentModelIndex&);
			void HandleChanged (const QModelIndex&);
			FlatTreeItem_ptr GetFolder (const QString&);
		private slots:
			void handleDataChanged (const QModelIndex&, const QModelIndex&);
			void handleModelReset ();
			void handleRowsInserted (const QModelIndex&, int, int);
			void handleRowsAboutToBeRemoved (const QModelIndex&, int, int);
		};
	};
};

#endif

