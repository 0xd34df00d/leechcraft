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

#include "flattofoldersproxymodel.h"
#include <QSet>
#include <QItemSelectionRange>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
	struct FlatTreeItem
	{
		QList<FlatTreeItem_ptr> C_;
		FlatTreeItem_ptr Parent_;

		enum Type
		{
			TRoot,
			TFolder,
			TItem
		};

		Type Type_;

		QPersistentModelIndex Index_;
		QString Tag_;

		int Row () const
		{
			if (Parent_)
			{
				QList<FlatTreeItem_ptr> c = Parent_->C_;
				for (int i = 0, size = Parent_->C_.size ();
						i < size; ++i)
					if (c.at (i).get () == this)
						return i;
			}
			return 0;
		}
	};

	FlatTreeItem* ToFlat (const QModelIndex& idx)
	{
		return static_cast<FlatTreeItem*> (idx.internalPointer ());
	}

	namespace Util
	{
		FlatToFoldersProxyModel::FlatToFoldersProxyModel (QObject *parent)
		: QAbstractItemModel (parent)
		, SourceModel_ (0)
		, TM_ (0)
		, Root_ (new FlatTreeItem)
		{
			Root_->Type_ = FlatTreeItem::TRoot;
		}

		void FlatToFoldersProxyModel::SetTagsManager (ITagsManager *tm)
		{
			TM_ = tm;
			reset ();
		}

		int FlatToFoldersProxyModel::columnCount (const QModelIndex&) const
		{
			return SourceModel_->columnCount (QModelIndex ());
		}

		QVariant FlatToFoldersProxyModel::data (const QModelIndex& index, int role) const
		{
			FlatTreeItem *fti = ToFlat (index);
			if (fti->Type_ == FlatTreeItem::TItem)
				return fti->Index_.data (role);
			else if (fti->Type_ == FlatTreeItem::TFolder &&
					index.column () == 0)
				return fti->Tag_;
			else
				return QVariant ();
		}

		QVariant FlatToFoldersProxyModel::headerData (int section,
				Qt::Orientation orient, int role) const
		{
			if (SourceModel_)
				return SourceModel_->headerData (section, orient, role);
			else
				return QVariant ();
		}

		Qt::ItemFlags FlatToFoldersProxyModel::flags (const QModelIndex& index) const
		{
			FlatTreeItem *fti = ToFlat (index);
			if (fti->Type_ == FlatTreeItem::TItem)
				return fti->Index_.flags ();
			else
				return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		}

		QModelIndex FlatToFoldersProxyModel::index (int row, int column,
				const QModelIndex& parent) const
		{
			FlatTreeItem *fti = 0;
			if (parent.isValid ())
				fti = ToFlat (parent);
			else
				fti = Root_.get ();

			if (fti->Type_ == FlatTreeItem::TItem)
				return QModelIndex ();
			else
				return createIndex (row, column, fti->C_.at (row).get ());
		}

		QModelIndex FlatToFoldersProxyModel::parent (const QModelIndex& index) const
		{
			FlatTreeItem *fti = 0;
			if (index.isValid ())
				fti = ToFlat (index);
			else
				fti = Root_.get ();

			FlatTreeItem_ptr parent;
			parent = fti->Parent_;

			if (parent &&
					parent->Type_ != FlatTreeItem::TRoot)
				return createIndex (parent->Row (), 0, parent.get ());
			else
				return QModelIndex ();
		}

		int FlatToFoldersProxyModel::rowCount (const QModelIndex& index) const
		{
			if (index.isValid ())
				return ToFlat (index)->C_.size ();
			else
				return Root_->C_.size ();
		}

		void FlatToFoldersProxyModel::SetSourceModel (QAbstractItemModel *model)
		{
			if (SourceModel_)
				disconnect (SourceModel_,
						0,
						this,
						0);

			SourceModel_ = model;

			// We don't support changing columns (yet) so don't connect
			// to columns* signals.
			connect (model,
					SIGNAL (headerDataChanged (Qt::Orientation, int, int)),
					this,
					SIGNAL (headerDataChanged (Qt::Orientation, int, int)));
			connect (model,
					SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
					this,
					SLOT (handleDataChanged (const QModelIndex&, const QModelIndex&)));
			connect (model,
					SIGNAL (layoutAboutToBeChanged ()),
					this,
					SIGNAL (layoutAboutToBeChanged ()));
			connect (model,
					SIGNAL (layoutChanged ()),
					this,
					SIGNAL (layoutChanged ()));
			connect (model,
					SIGNAL (modelReset ()),
					this,
					SLOT (handleModelReset ()));
			connect (model,
					SIGNAL (rowsInserted (const QModelIndex&,
							int, int)),
					this,
					SLOT (handleRowsInserted (const QModelIndex&,
							int, int)));
			connect (model,
					SIGNAL (rowsAboutToBeRemoved (const QModelIndex&,
							int, int)),
					this,
					SLOT (handleRowsAboutToBeRemoved (const QModelIndex&,
							int, int)));

			Items_.clear ();
			Root_->C_.clear ();
			handleModelReset ();
		}

		QModelIndex FlatToFoldersProxyModel::MapToSource (const QModelIndex& proxy) const
		{
			if (!proxy.isValid ())
				return QModelIndex ();

			FlatTreeItem *item = ToFlat (proxy);

			if (item->Type_ != FlatTreeItem::TItem)
				return QModelIndex ();

			return item->Index_;
		}

		FlatTreeItem_ptr FlatToFoldersProxyModel::GetFolder (const QString& tag)
		{
			QList<FlatTreeItem_ptr>& c = Root_->C_;
			Q_FOREACH (FlatTreeItem_ptr item, c)
				if (item->Tag_ == tag)
					return item;

			FlatTreeItem_ptr item (new FlatTreeItem);
			item->Type_ = FlatTreeItem::TFolder;
			item->Tag_ = tag;
			item->Parent_ = Root_;

			int size = c.size ();
			beginInsertRows (QModelIndex (), size, size);
			c.append (item);
			endInsertRows ();

			return item;
		}

		void FlatToFoldersProxyModel::HandleRowInserted (int i)
		{
			QModelIndex idx = SourceModel_->index (i, 0);

			QStringList tags = idx.data (RoleTags).toStringList ();

			if (tags.isEmpty ())
				tags << QString (tr ("untagged"));

			QPersistentModelIndex pidx (idx);

			Q_FOREACH (QString tag, tags)
				AddForTag (tag, pidx);
		}

		void FlatToFoldersProxyModel::HandleRowRemoved (int i)
		{
			QAbstractItemModel *model = SourceModel_;
			QModelIndex idx = model->index (i, 0);

			QStringList tags = idx.data (RoleTags).toStringList ();

			if (tags.isEmpty ())
				tags << QString (tr ("untagged"));

			QPersistentModelIndex pidx (idx);

			Q_FOREACH (QString tag, tags)
				RemoveFromTag (tag, pidx);
		}

		void FlatToFoldersProxyModel::AddForTag (const QString& tag,
				const QPersistentModelIndex& pidx)
		{
			FlatTreeItem_ptr folder = GetFolder (tag);

			FlatTreeItem_ptr item (new FlatTreeItem);
			item->Type_ = FlatTreeItem::TItem;
			item->Index_ = pidx;
			item->Parent_ = folder;

			int size = folder->C_.size ();
			QModelIndex iidx = index (Root_->C_.indexOf (folder), 0);
			beginInsertRows (iidx, size, size);
			folder->C_.append (item);
			Items_.insert (pidx, item);
			endInsertRows ();
		}

		void FlatToFoldersProxyModel::RemoveFromTag (const QString& tag,
				const QPersistentModelIndex& pidx)
		{
			FlatTreeItem_ptr folder = GetFolder (tag);
			QList<FlatTreeItem_ptr>& c = folder->C_;
			for (int i = 0, size = c.size ();
					i < size; ++i)
			{
				if (c.at (i)->Index_ != pidx)
					continue;

				beginRemoveRows (index (Root_->C_.indexOf (folder), 0), i, i);
				Items_.remove (pidx, c.at (i));
				c.removeAt (i);
				endRemoveRows ();
				break;
			}
		}

		void FlatToFoldersProxyModel::HandleChanged (const QModelIndex& idx)
		{
			QSet<QString> newTags = QSet<QString>::fromList (idx.data (RoleTags).toStringList ());

			QPersistentModelIndex pidx (idx);
			QList<FlatTreeItem_ptr> items = Items_.values (pidx);

			QSet<QString> oldTags;
			Q_FOREACH (FlatTreeItem_ptr item, items)
				oldTags << item->Tag_;

			QSet<QString> added = QSet<QString> (newTags).subtract (oldTags);
			QSet<QString> removed = QSet<QString> (oldTags).subtract (newTags);

			Q_FOREACH (QString rem, removed)
				RemoveFromTag (rem, pidx);

			Q_FOREACH (QString add, added)
				AddForTag (add, pidx);
		}

		void FlatToFoldersProxyModel::handleDataChanged (const QModelIndex& topLeft,
				const QModelIndex& bottomRight)
		{
			QItemSelectionRange range (topLeft, bottomRight);
			QModelIndexList indexes = range.indexes ();
			for (int i = 0, size = indexes.size ();
					i < size; ++i)
				HandleChanged (indexes.at (i));
		}

		void FlatToFoldersProxyModel::handleModelReset ()
		{
			if (Root_->C_.size ())
			{
				beginRemoveRows (QModelIndex (), 0, Root_->C_.size () - 1);
				Root_->C_.clear ();
				Items_.clear ();
				endRemoveRows ();
			}

			for (int i = 0, size = SourceModel_->rowCount ();
					i < size; ++i)
				HandleRowInserted (i);

			reset ();
		}

		void FlatToFoldersProxyModel::handleRowsInserted (const QModelIndex&,
				int start, int end)
		{
			for (int i = start; i <= end; ++i)
				HandleRowInserted (i);
		}

		void FlatToFoldersProxyModel::handleRowsAboutToBeRemoved (const QModelIndex&,
				int start, int end)
		{
			for (int i = start; i <= end; ++i)
				HandleRowRemoved (i);
		}
	};
};

