/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include "modelsconfig.h"

namespace LC::Util
{
	/** @brief Proxy model flattening a hierarchical model.
	 *
	 * This model takes another model and folds its structure into a flat
	 * model, possibly filtering out some rows (via the IsIndexAccepted()
	 * method).
	 *
	 * Since this model changes the structure of the source model quite a
	 * lot, it doesn't derive from QAbstractProxyModel.
	 *
	 * @ingroup ModelUtil
	 */
	class UTIL_MODELS_API FlattenFilterModel : public QAbstractItemModel
	{
	protected:
		QAbstractItemModel *Source_ = nullptr;
		QList<QPersistentModelIndex> SourceIndexes_;
	public:
		/** @brief Constructs the model with the given \em parent.
		 *
		 * @param[in] parent The parent object of this model.
		 */
		using QAbstractItemModel::QAbstractItemModel;

		/** @brief Reimplemented from QAbstractItemModel.
		 */
		QModelIndex index (int, int, const QModelIndex& = {}) const override;

		/** @brief Reimplemented from QAbstractItemModel.
		 */
		QModelIndex parent (const QModelIndex&) const override;

		/** @brief Reimplemented from QAbstractItemModel.
		 */
		int rowCount (const QModelIndex& parent = {}) const override;

		/** @brief Reimplemented from QAbstractItemModel.
		 */
		int columnCount (const QModelIndex& parent = {}) const override;

		/** @brief Reimplemented from QAbstractItemModel.
		 */
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;

		/** @brief Sets the source model to \em model.
		 *
		 * If another source model has been set already, this function
		 * rebuilds the whole model, effectively resetting it.
		 *
		 * @param[in] model The new source model.
		 */
		void SetSource (QAbstractItemModel *model);
	protected:
		/** @brief Checks whether the given \em index should be included
		 * in the model.
		 *
		 * Reimplement this function in derived classes to provide
		 * filtering capabilities akin to QSortFilterProxyModel.
		 *
		 * The children of the \em index will be checked even if this
		 * function returns false for \em index.
		 *
		 * @note The model operates on rows: that is, only the first
		 * column is checked, and <code>index.column() == 0</code>
		 * will always hold.
		 *
		 * The default implementation simply returns <code>true</code>.
		 *
		 * @param[in] index The index of the source model to check.
		 * @return Whether the \em index should be included in the
		 * resulting model.
		 */
		virtual bool IsIndexAccepted (const QModelIndex& index) const;
	private:
		void HandleDataChanged (const QModelIndex&, const QModelIndex&);
		void HandleRowsInserted (const QModelIndex&, int, int);
		void HandleRowsAboutRemoved (const QModelIndex&, int, int);
	};
}
