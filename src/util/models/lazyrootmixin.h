/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <concepts>
#include <functional>
#include <utility>
#include <QAbstractItemModel>

namespace LC::Util
{
	/** @brief Defers populating the model until a view first asks for rows.
	 *
	 * Derive from this class (or instantiate it directly), parametrizing
	 * the template by the desired base class like `QStandardItemModel`, and
	 * pass the function populating the model. The function is invoked when
	 * an attached view first asks the root for more rows, and on each
	 * `Reload()` after that, always on an empty root: the existing rows are
	 * removed first, so the base model must support `removeRows()` on the
	 * root.
	 *
	 * Only views (and proxies in front of them) drive the loading: code
	 * reading the model directly sees it empty until then and should call
	 * `EnsureLoaded()` first.
	 *
	 * @tparam Model The model type to derive from.
	 *
	 * @ingroup ModelUtil
	 */
	template<std::derived_from<QAbstractItemModel> Model>
	class LazyRootMixin : public Model
	{
		const std::function<void ()> Load_;
		bool Loaded_ = false;
	public:
		/** @brief Constructs the model passing the arguments to the base
		 * constructor.
		 *
		 * @param[in] load The function populating the empty model.
		 * @param[in] args The list of parameters to pass to the
		 * constructor of the base `Model`.
		 */
		template<typename... Args>
		explicit LazyRootMixin (std::function<void ()> load, Args&&... args)
		: Model { std::forward<Args> (args)... }
		, Load_ { std::move (load) }
		{
		}

		/** @brief Returns whether the model has been populated.
		 */
		bool IsLoaded () const
		{
			return Loaded_;
		}

		/** @brief Populates the model if it hasn't been populated yet.
		 */
		void EnsureLoaded ()
		{
			if (!Loaded_)
				Reload ();
		}

		/** @brief Drops the rows and populates the model anew.
		 */
		void Reload ()
		{
			Loaded_ = true;
			this->removeRows (0, this->rowCount ());
			Load_ ();
		}

		bool hasChildren (const QModelIndex& parent) const override
		{
			return IsUnloadedRoot (parent) || Model::hasChildren (parent);
		}

		bool canFetchMore (const QModelIndex& parent) const override
		{
			return IsUnloadedRoot (parent) || Model::canFetchMore (parent);
		}

		void fetchMore (const QModelIndex& parent) override
		{
			if (IsUnloadedRoot (parent))
				Reload ();
			else
				Model::fetchMore (parent);
		}
	private:
		bool IsUnloadedRoot (const QModelIndex& parent) const
		{
			return !parent.isValid () && !Loaded_;
		}
	};
}
