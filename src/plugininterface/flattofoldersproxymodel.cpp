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
#include <interfaces/iinfo.h>

namespace LeechCraft
{
	struct ChildLevel
	{
		QPersistentModelIndex Index_;
	};

	struct FolderLevel
	{
		QString Tag_;
		QList<ChildLevel_ptr> Childs_;
	};

	namespace Util
	{
		FlatToFoldersProxyModel::FlatToFoldersProxyModel (QObject *parent)
		: QAbstractProxyModel (parent)
		, TM_ (0)
		{
		}

		void FlatToFoldersProxyModel::SetTagsManager (ITagsManager *tm)
		{
			TM_ = tm;
			reset ();
		}

		int FlatToFoldersProxyModel::columnCount (const QModelIndex&) const
		{
			return sourceModel ()->columnCount (QModelIndex ());
		}

		QVariant FlatToFoldersProxyModel::data (const QModelIndex& index, int role) const
		{
			return QVariant ();
		}

		Qt::ItemFlags FlatToFoldersProxyModel::flags (const QModelIndex& index) const
		{
			if (index.parent ().isValid ())
				return mapToSource (index).flags ();
			else
				return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		}

		QModelIndex FlatToFoldersProxyModel::index (int row, int column, const QModelIndex& parent) const
		{
			if (parent.isValid ())
				return QModelIndex ();
			else
			{
				if (!hasIndex (row, column))
					return QModelIndex ();

				return createIndex (row, column, row - 1);
			}
		}

		QModelIndex FlatToFoldersProxyModel::parent (const QModelIndex& index) const
		{
			return QModelIndex ();
		}

		int FlatToFoldersProxyModel::rowCount (const QModelIndex& index) const
		{
			if (!TM_)
				return 0;

			if (index.isValid ())
			{
			}
			else
				// + 1 for the "tagless" items
				return CurrentTags_.size () + 1;
		}

		void FlatToFoldersProxyModel::setSourceModel (QAbstractItemModel *model)
		{
			QAbstractProxyModel::setSourceModel (model);
			
			disconnect (model,
					0,
					this,
					0);
		}

		void FlatToFoldersProxyModel::handleModelReset ()
		{
			QAbstractItemModel *model = sourceModel ();

			Folders_.clear ();

			for (int i = 0, size = model->rowCount ();
					i < size; ++i)
			{
				QModelIndex index = model->index (i, 0);

				QStringList tags = index.data (RoleTags).toStringList ();

				Q_FOREACH (QString tag, tags)
				{
					if (!Tag2Folder_.contains (tag))
					{
						FolderLevel_ptr folder (new FolderLevel);
						folder->Tag_ = tag;
						Tag2Folder_ [tag] = folder;
					}

					QPersistentModelIndex pidx (index);
					if (!Index2Child_.contains (pidx))
					{
						ChildLevel_ptr child (new ChildLevel);
						child->Index_ = pidx;

						Index2Child_ [pidx] = child;
						Tag2Folder_ [tag]->Childs_ << child;
					}
				}
			}

			reset ();
		}
	};
};

