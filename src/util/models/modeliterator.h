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

namespace LC::Util
{
	/** @brief Provides an iterator-based API to a Qt model.
	 *
	 * This class wraps a subclass of QAbstractItemModel into an
	 * STL-iterator-friendly API. The provided iterator is of the random
	 * access category.
	 *
	 * A model can be traversed either by rows (incrementing row index at
	 * each iterator increment) or by columns (incrementing column index
	 * at each iterator increment). The Direction enumeration controls
	 * the exact behavior. The exact index which is changed during
	 * iterating is called the \em traversable.
	 *
	 * @ingroup ModelUtil
	 */
	class UTIL_MODELS_API ModelIterator
	{
		QAbstractItemModel * const Model_;
		const QModelIndex Parent_;

		int Row_;
		int Col_;
	public:
		/** @brief The direction of traversal.
		 */
		enum class Direction
		{
			/** @brief The model should be traversed by rows.
			 *
			 * Thus, the traversable is the row index.
			 */
			Rows,

			/** @brief The model should be traversed by columns.
			 *
			 * Thus, the traversable is the column index.
			 */
			Cols
		};
	private:
		const Direction Dir_;
	public:
		/** @brief Constructs an iterator.
		 *
		 * @param[in] model The model to iterate over.
		 * @param[in] row The starting row of the iterator.
		 * @param[in] col The starting column of the iterator.
		 * @param[in] dir The direction of traversal.
		 * @param[in] parent The parent index to be used during traversal.
		 */
		ModelIterator (QAbstractItemModel *model, int row, int col = 0,
				Direction dir = Direction::Rows, const QModelIndex& parent = {});

		/** @brief Increments the traversable index and returns the
		 * modified iterator.
		 *
		 * @return The modified iterator with the new traversable index.
		 */
		ModelIterator& operator++ ();

		/** @brief Returns the current iterator and increments the
		 * traversable index.
		 *
		 * @return The iterator with the old traversable index.
		 */
		ModelIterator operator++ (int);

		/** @brief Decrements the traversable index and returns the
		 * modified iterator.
		 *
		 * @return The modified iterator with the new traversable index.
		 */
		ModelIterator& operator-- ();

		/** @brief Returns the current iterator and decrements the
		 * traversable index.
		 *
		 * @return The iterator with the old traversable index.
		 */
		ModelIterator operator-- (int);

		/** @brief Adds the given \em count to the traversable index.
		 *
		 * @param[in] count The number to add to the current traversable.
		 * @return The modified iterator with the new traversable index.
		 */
		ModelIterator& operator+= (int count);

		/** @brief Subtracts the given \em count to the traversable index.
		 *
		 * @param[in] count The number to subtract from the current
		 * traversable.
		 * @return The modified iterator with the new traversable index.
		 */
		ModelIterator& operator-= (int count);

		/** @brief Computes the distance between this and another iterator.
		 *
		 * The distance is the different between traversable indexes of
		 * this and \em other iterator.
		 *
		 * If the iterators have different traverse directions, the
		 * behavior is undefined.
		 *
		 * @param[in] other The other iterator to compute distance with.
		 * @return The distance between this and \em other iterator, which
		 * may be negative.
		 */
		int operator- (const ModelIterator& other) const;

		/** @brief Checks if two iterators are equal.
		 *
		 * For iterators to be equal the model, parent index, row and
		 * column should all be equal. Traversal index is \em not taken
		 * into account.
		 *
		 * @param[in] left First iterator to check for equality.
		 * @param[in] right Second iterator to check for equality.
		 * @return Whether \em left and \em right are equal.
		 */
		friend UTIL_MODELS_API bool operator== (const ModelIterator& left, const ModelIterator& right);

		/** @brief Returns the index currently pointed by the iterator.
		 *
		 * @return The index defined by the parent index of this iterator
		 * and the current (row, column) pair.
		 */
		QModelIndex operator* () const;

		/** @brief Returns the current row.
		 *
		 * @return The current row of the iterator.
		 *
		 * @sa GetCol()
		 */
		int GetRow () const;

		/** @brief Returns the current column.
		 *
		 * @return The current column of the iterator.
		 *
		 * @sa GetRow()
		 */
		int GetCol () const;
	private:
		int& GetIncrementable ();
		int GetIncrementable () const;
	};
}

namespace std
{
	template<>
	struct iterator_traits<LC::Util::ModelIterator>
	{
		typedef QModelIndex value_type;
		typedef int difference_type;

		typedef random_access_iterator_tag iterator_category;
	};
}
