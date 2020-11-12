/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QVector>

namespace LC::Util
{
	/** @brief Base class for model items for tree-like models.
	 *
	 * A ModelItemBase only manages its children items and keeps track of
	 * its parent item. It does not contain any data itself.
	 *
	 * This class is intended to be a base class for some concreted model
	 * item implementation (see ModelItem class for an example). In
	 * particular it cannot be instantiated itself since its constructors
	 * are protected.
	 *
	 * The lifetime of the items is also managed. Shared pointers are
	 * used for this: the ModelItemBase stores a list of shared pointers
	 * to its children and a weak pointer to its parent.
	 *
	 * In particular, child items are automatically deleted when they are
	 * removed from the children list of an item via EraseChild() or
	 * EraseChildren() functions, unless a shared pointer reference is
	 * kept to a child. A new child can be added via AppendChild(),
	 * InsertChild() or AppendExisting().
	 *
	 * An STL iterator-like interface to the children is also provided,
	 * so a ModelItemBase can also be used in a range-for loop.
	 *
	 * @tparam T The type of the class derived from ModelItemBase.
	 *
	 * @sa ModelItem
	 *
	 * @ingroup ModelUtil
	 */
	template<typename T>
	class ModelItemBase : public std::enable_shared_from_this<T>
	{
	protected:
		using T_wptr = std::weak_ptr<T>;
		using T_ptr = std::shared_ptr<T>;
		using T_cptr = std::shared_ptr<const T>;
		using TList_t = QVector<T_ptr>;

		T_wptr Parent_;
		TList_t Children_;

		/** @brief Constructs a default ModelItemBase with no parent.
		 */
		ModelItemBase () = default;

		/** @brief Constructs a ModelItemBase with a given parent item.
		 *
		 * @param[in] parent The weak reference to the parent item.
		 */
		explicit ModelItemBase (const T_wptr& parent)
		: Parent_ { parent }
		{
		}
	public:
		/** @brief A non-const iterator for the list of children.
		 */
		using iterator = typename TList_t::iterator;

		/** @brief A const iterator for the list of children.
		 */
		using const_iterator = typename TList_t::const_iterator;

		/** @brief Returns a non-const iterator pointing to the beginning
		 * of the child items list.
		 *
		 * @return A non-const iterator pointing to the beginning of the children
		 * list.
		 */
		iterator begin ()
		{
			return Children_.begin ();
		}

		/** @brief Returns a non-const iterator pointing past the last
		 * child item.
		 *
		 * @return A non-const iterator pointing to the past the last child item.
		 */
		iterator end ()
		{
			return Children_.end ();
		}

		/** @brief Returns a const iterator pointing to the beginning
		 * of the child items list.
		 *
		 * @return A const iterator pointing to the beginning of the children
		 * list.
		 */
		const_iterator begin () const
		{
			return Children_.begin ();
		}

		/** @brief Returns a const iterator pointing past the last
		 * child item.
		 *
		 * @return A const iterator pointing to the past the last child item.
		 */
		const_iterator end () const
		{
			return Children_.end ();
		}

		/** @brief Returns a child at the given \em row.
		 *
		 * If there is no child at the given row, returns a null
		 * shared_ptr.
		 *
		 * @param[in] row The index of the child to be returned.
		 * @return A pointer to the child in the \em row, or a null
		 * pointer if there is no child at that row.
		 */
		T_ptr GetChild (int row) const
		{
			return Children_.value (row);
		}

		/** @brief Returns a constant reference to the list of children.
		 *
		 * @return The constant reference to the list of children.
		 */
		const TList_t& GetChildren () const
		{
			return Children_;
		}

		/** @brief Returns a non-constant reference to the list of children.
		 *
		 * @return The non-constant reference to the list of children.
		 */
		TList_t& GetChildren ()
		{
			return Children_;
		}

		/** @brief Returns the children count.
		 *
		 * @return The children count.
		 */
		int GetRowCount () const
		{
			return Children_.size ();
		}

		/** @brief Returns whether there are any children at all.
		 *
		 * @return Returns true if there are no child items, or false
		 * otherwise.
		 */
		bool IsEmpty () const
		{
			return Children_.isEmpty ();
		}

		/** @brief Erases a child item at the position defined by \em it.
		 *
		 * If \em it is invalid iterator, the behavior is undefined.
		 *
		 * This function invalidates all iterators.
		 *
		 * @param[in] it The iterator pointing to the child to be erased.
		 * @return The iterator pointing to the item right next to the
		 * erased one.
		 *
		 * @sa EraseChildren()
		 */
		iterator EraseChild (iterator it)
		{
			return Children_.erase (it);
		}

		/** @brief Erases all child items in the given range.
		 *
		 * The range is half-open, comprised of items in the
		 * <code>[begin; end)</code> range of iterators.
		 *
		 * If the range contains invalid iterators, the behavior is
		 * undefined.
		 *
		 * This function invalidates all iterators.
		 *
		 * @param[in] begin The iterator pointing to the first item to be
		 * erased.
		 * @param[in] end The iterator pointing past the last item to be
		 * erased.
		 * @return The iterator pointing to the item right next to the
		 * last erased one.
		 *
		 * @sa EraseChild()
		 */
		iterator EraseChildren (iterator begin, iterator end)
		{
			return Children_.erase (begin, end);
		}

		/** @brief Appends a child item \em t to the list of child items.
		 *
		 * This function invalidates all iterators.
		 *
		 * @param[in] t The child item to append.
		 *
		 * @sa AppendChild()
		 * @sa InsertChild()
		 */
		void AppendExisting (const T_ptr& t)
		{
			Children_ << t;
		}

		/** @brief Appends a list of \em items to the list of child items.
		 *
		 * This function invalidates all iterators.
		 *
		 * @param[in] items The list of items to append.
		 *
		 * @sa AppendChild()
		 * @sa InsertChild()
		 */
		void AppendExisting (const TList_t& items)
		{
			Children_ += items;
		}

		/** @brief Creates a new child item, appends it and returns it.
		 *
		 * @tparam Args The types of the arguments to the constructor of
		 * of the model.
		 * @param[in] args The arguments to the constructor of the model
		 * item type \em T.
		 * @return The pointer to the newly created child item.
		 *
		 * @sa AppendExisting()
		 * @sa InsertChild()
		 */
		template<typename... Args>
		T_ptr& AppendChild (Args&&... args)
		{
			Children_.append (std::make_shared<T> (std::forward<Args> (args)...));
			return Children_.last ();
		}

		/** @brief Creates a new child item, inserts it at the given
		 * position and returns it.
		 *
		 * @tparam Args The types of the arguments to the constructor of
		 * of the model.
		 * @param[in] pos The index where the newly created item should be
		 * inserted.
		 * @param[in] args The arguments to the constructor of the model
		 * item type \em T.
		 * @return The pointer to the newly created child item.
		 *
		 * @sa AppendExisting()
		 * @sa AppendChild()
		 */
		template<typename... Args>
		T_ptr& InsertChild (int pos, Args&&... args)
		{
			Children_.insert (pos, std::make_shared<T> (std::forward<Args> (args)...));
			return Children_ [pos];
		}

		/** @brief Returns the pointer to the parent item.
		 *
		 * @return The pointer to the parent item or a null pointer if
		 * the parent item is deleted already or if there never was a
		 * parent.
		 */
		T_ptr GetParent () const
		{
			return Parent_.lock ();
		}

		/** @brief Returns the index of the \em item in the children list.
		 *
		 * @return The index of the \em item in the children list, of -1
		 * if \em item is not a child of this model item.
		 */
		int GetRow (const T_ptr& item) const
		{
			return Children_.indexOf (item);
		}

		/** @brief Returns the index of the \em item in the children list.
		 *
		 * @return The index of the \em item in the children list, of -1
		 * if \em item is not a child of this model item.
		 */
		int GetRow (const T_cptr& item) const
		{
			const auto pos = std::find (Children_.begin (), Children_.end (), item);
			return pos == Children_.end () ?
					-1 :
					std::distance (Children_.begin (), pos);
		}

		/** @brief Returns the index of this item in the parent's
		 * children list.
		 *
		 * If the parent is already deleted or there is no parent, -1 is
		 * returned.
		 *
		 * @return The index of this item in the parent's list, or -1 if
		 * there is no parent.
		 */
		int GetRow () const
		{
			const auto parent = GetParent ();
			if (!parent)
				return -1;
			return parent->GetRow (this->shared_from_this ());
		}
	};
}
