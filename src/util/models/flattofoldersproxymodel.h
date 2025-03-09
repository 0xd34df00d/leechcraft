/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QAbstractItemModel>
#include <QStringList>
#include <QMultiHash>
#include "modelsconfig.h"

class ITagsManager;

namespace LC::Util
{
	struct FlatTreeItem;
	using FlatTreeItem_ptr = std::shared_ptr<FlatTreeItem>;

	class UTIL_MODELS_API FlatToFoldersProxyModel : public QAbstractItemModel
	{
		QAbstractItemModel *SourceModel_ = nullptr;

		const ITagsManager * const TM_;

		FlatTreeItem_ptr Root_;
		QMultiHash<QPersistentModelIndex, FlatTreeItem_ptr> Items_;
	public:
		explicit FlatToFoldersProxyModel (const ITagsManager*, QObject* = nullptr);

		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;

		Qt::DropActions supportedDropActions () const override;
		QStringList mimeTypes () const override;
		QMimeData* mimeData (const QModelIndexList& indexes) const override;
		bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

		void SetSourceModel (QAbstractItemModel*);
		QAbstractItemModel* GetSourceModel () const;
		QModelIndex MapToSource (const QModelIndex&) const;
		QList<QModelIndex> MapFromSource (const QModelIndex&) const;

		bool IsFolder (const QModelIndex&) const;
		QList<QModelIndex> GetChildren (const QModelIndex&) const;
		QList<QVariant> GetChildrenData (const QModelIndex& index, int role) const;

		template<typename T>
		QList<T> GetChildrenData (const QModelIndex& index, int role) const
		{
			QList<T> result;
			for (const auto& var : GetChildrenData (index, role))
				result.push_back (var.value<T> ());
			return result;
		}
	private:
		const FlatTreeItem& ToFlatOrRoot (const QModelIndex&) const;

		void HandleRowInserted (int);
		void HandleRowRemoved (int);
		void AddForTag (const QString&, const QPersistentModelIndex&);
		void RemoveFromTag (const QString&, const QPersistentModelIndex&);
		void HandleChanged (const QModelIndex&);

		FlatTreeItem_ptr FindFolder (const QString&) const;
		FlatTreeItem_ptr GetFolder (const QString&);

		void HandleDataChanged (const QModelIndex&, const QModelIndex&);
		void HandleModelReset ();
		void HandleRowsInserted (const QModelIndex&, int, int);
		void HandleRowsAboutToBeRemoved (const QModelIndex&, int, int);
	};
}
