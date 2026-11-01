/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <QAbstractItemModel>
#include <QStringList>
#include <QMultiHash>
#include "modelsconfig.h"

namespace LC::Util
{
	struct FlatTreeItem;
	using FlatTreeItem_ptr = std::shared_ptr<FlatTreeItem>;

	/** Groups the rows of a flat source model into folders by key.
	 *
	 * `Config::KeysOf_` yields the keys of a source row from its column 0
	 * index, and the row appears under the folder of each of them, a key
	 * listed twice counting once. Rows with no keys go into a single unkeyed
	 * folder. A folder shows its key as `Qt::DisplayRole`, the unkeyed one
	 * shows `Config::UnkeyedLabel_`. Leaves expose the columns of their
	 * source rows and map back via `MapToSource()`.
	 *
	 * A drop onto a folder reaches the source model's `dropMimeData()` with
	 * the folder's key under the `Config::DropFormat_` format, empty for the
	 * unkeyed folder.
	 *
	 * When the keys of the rows change without the source announcing it,
	 * `ReplaceKey()` moves the rows along.
	 *
	 * The source must be flat, and its layout changes must move rows as units,
	 * as every row sort does. Layout changes of the source are not forwarded,
	 * and a source rearranging cells independently of each other is not
	 * supported.
	 *
	 * @ingroup ModelUtil
	 */
	class UTIL_MODELS_API FlatToFoldersProxyModel : public QAbstractItemModel
	{
	public:
		struct Config
		{
			/** The keys of a source row, none of them empty.
			 */
			std::function<QStringList (const QModelIndex&)> KeysOf_;
			QString UnkeyedLabel_;
			QString DropFormat_;
		};
	private:
		const Config Config_;

		QAbstractItemModel *SourceModel_ = nullptr;

		FlatTreeItem_ptr Root_;
		QMultiHash<QPersistentModelIndex, FlatTreeItem_ptr> Items_;
	public:
		explicit FlatToFoldersProxyModel (Config, QObject* = nullptr);

		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole) override;
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

		/** @brief Updates the folders after the key `old` was replaced with
		 * `newKeys` on every row that had it, the other keys of the rows
		 * being unchanged.
		 *
		 * A single new key without a folder yet renames the folder of `old`
		 * in place, keeping its rows; otherwise the rows move to the folders
		 * of `newKeys`.
		 */
		void ReplaceKey (const QString& old, const QStringList& newKeys);
	private:
		QStringList KeysOf (const QModelIndex&) const;
		const FlatTreeItem& ToFlatOrRoot (const QModelIndex&) const;
		QModelIndex IndexOf (const FlatTreeItem&, int column = 0) const;

		FlatTreeItem_ptr FindFolder (const QString&) const;
		FlatTreeItem_ptr GetFolder (const QString&);
		void AddForKey (const QString&, const QPersistentModelIndex&);
		void RemoveLeaf (const FlatTreeItem_ptr&);
		void UpdateLeaves (const QPersistentModelIndex&, const QList<int>& roles);
		void RenameFolder (FlatTreeItem&, const QString&);

		void HandleDataChanged (const QModelIndex&, const QModelIndex&, const QList<int>&);
		void HandleModelReset ();
		void HandleRowsInserted (const QModelIndex&, int, int);
		void HandleRowsAboutToBeRemoved (const QModelIndex&, int, int);
	};
}
