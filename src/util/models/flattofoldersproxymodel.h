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

#ifndef UTIL_FLATTOFOLDERSPROXYMODEL_H
#define UTIL_FLATTOFOLDERSPROXYMODEL_H
#include <memory>
#include <QAbstractItemModel>
#include <QStringList>
#include <QMultiHash>
#include <util/utilconfig.h>

class ITagsManager;

namespace LeechCraft
{
	struct FlatTreeItem;
	typedef std::shared_ptr<FlatTreeItem> FlatTreeItem_ptr;

	namespace Util
	{
		class UTIL_API FlatToFoldersProxyModel : public QAbstractItemModel
		{
			Q_OBJECT

			QAbstractItemModel *SourceModel_;

			ITagsManager *TM_;

			FlatTreeItem_ptr Root_;
			QMultiHash<QPersistentModelIndex, FlatTreeItem_ptr> Items_;
		public:
			FlatToFoldersProxyModel (QObject* = 0);

			void SetTagsManager (ITagsManager*);

			int columnCount (const QModelIndex& = QModelIndex ()) const;
			QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
			QVariant headerData (int, Qt::Orientation, int) const;
			Qt::ItemFlags flags (const QModelIndex&) const;
			QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
			QModelIndex parent (const QModelIndex&) const;
			int rowCount (const QModelIndex& = QModelIndex ()) const;

			Qt::DropActions supportedDropActions () const;
			QStringList mimeTypes () const;
			QMimeData* mimeData (const QModelIndexList& indexes) const;
			bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

			void SetSourceModel (QAbstractItemModel*);
			QAbstractItemModel* GetSourceModel () const;
			QModelIndex MapToSource (const QModelIndex&) const;
			QList<QModelIndex> MapFromSource (const QModelIndex&) const;
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

