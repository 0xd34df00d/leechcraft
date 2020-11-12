/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QModelIndex>
#include "modelsconfig.h"
#include "modelitembase.h"

namespace LC::Util
{
	class ModelItem;

	typedef std::shared_ptr<ModelItem> ModelItem_ptr;
	typedef std::weak_ptr<ModelItem> ModelItem_wtr;
	typedef QVector<ModelItem_ptr> ModelItemsList_t;
	typedef std::shared_ptr<const ModelItem> ModelItem_cptr;

	/** @brief Provides a proxying API on top of an QAbstractItemModel.
	 *
	 * This class simplifies writing wrappers around QAbstractItemModel
	 * classes representing the data in different means than Qt's MVC.
	 *
	 * @ingroup ModelUtil
	 */
	class UTIL_MODELS_API ModelItem final : public ModelItemBase<ModelItem>
	{
		QAbstractItemModel * const Model_ = nullptr;
		QModelIndex SrcIdx_;
	public:
		typedef ModelItemsList_t::iterator iterator;
		typedef ModelItemsList_t::const_iterator const_iterator;

		/** @brief Constructs a default (invalid) ModelItem having no
		 * model set.
		 */
		ModelItem () = default;

		/** @brief Constructs a ModelItem associated with a given
		 * \em index in the \em model.
		 *
		 * @param[in] model The model which this model item wraps.
		 * @param[in] index The source index in this model.
		 * @param[in] parent The parent ModelItem, or an empty pointer if
		 * this is the root item.
		 */
		ModelItem (QAbstractItemModel *model, const QModelIndex& index, const ModelItem_wtr& parent);

		/** @brief Ensures there is a child item at the given \em row.
		 *
		 * If necessary, this function expands the list of children until
		 * there is a given \em row.
		 *
		 * If no model item has been set for the given row, this function
		 * creates a proper ModelItem.
		 *
		 * @param[in] row The row for which to obtain a child item.
		 * @return A child item already existing at the given \em row or
		 * a new child item if there was no item at the \em row.
		 */
		ModelItem* EnsureChild (int row);

		/** @brief Returns the index this ModelItem instance wraps.
		 *
		 * @return The model index this item wraps, either the one passed
		 * to the constructor or the one obtained during the
		 * RefreshIndex() call.
		 */
		const QModelIndex& GetIndex () const;

		/** @brief Returns the wrapped model.
		 *
		 * @return The wrapped model.
		 */
		QAbstractItemModel* GetModel () const;

		/** @brief Updates the wrapped index so that it points at the
		 * given row.
		 *
		 * Calling this function on a ModelItem wrapping the root of the
		 * underlying model (that is, constructed by the parameterless
		 * constructor) leads to undefined behavior.
		 *
		 * @param[in] modelStartingRow The new row (among the children of
		 * this item's wrapped index) in the model this item should
		 * represent.
		 */
		void RefreshIndex (int modelStartingRow);

		/** @brief Finds a child item for the given \em index.
		 *
		 * The \em index is assumed to be the child of the one wrapped by
		 * this ModelItem.
		 *
		 * @param[in] index The index (a child of the one returned by
		 * GetIndex()) for which a child model item should be found.
		 * @return The model item wrapping the given \em index or a null
		 * pointer if there is no such item.
		 */
		ModelItem_ptr FindChild (QModelIndex index) const;
	};
}
