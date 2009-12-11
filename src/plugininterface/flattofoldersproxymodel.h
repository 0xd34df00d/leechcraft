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
#include <QAbstractProxyModel>
#include <QStringList>
#include "config.h"

class ITagsManager;

namespace LeechCraft
{
	struct FolderLevel;
	struct ChildLevel;

	typedef boost::shared_ptr<FolderLevel> FolderLevel_ptr;
	typedef boost::shared_ptr<ChildLevel> ChildLevel_ptr;

	namespace Util
	{
		class PLUGININTERFACE_API FlatToFoldersProxyModel : public QAbstractProxyModel
		{
			Q_OBJECT

			ITagsManager *TM_;
			QStringList CurrentTags_;
			QList<FolderLevel_ptr> Folders_;
			QMap<QString, FolderLevel_ptr> Tag2Folder_;
			QMap<QPersistentModelIndex, ChildLevel_ptr> Index2Child_;
		public:
			FlatToFoldersProxyModel (QObject* = 0);

			void SetTagsManager (ITagsManager*);

			virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
			virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
			virtual Qt::ItemFlags flags (const QModelIndex&) const;
			virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
			virtual QModelIndex parent (const QModelIndex&) const;
			virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

			virtual void setSourceModel (QAbstractItemModel*);
		private slots:
			void handleModelReset ();
		};
	};
};

#endif

