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
	/** @brief Defers populating the model until a view first shows it.
	 *
	 * Derive from this class (or instantiate it directly), parametrizing
	 * the template by the desired base class like QStandardItemModel, and
	 * pass the function populating the model. The function is invoked at
	 * most once, when an attached view first asks the root for more rows,
	 * which happens on the view's first layout, that is, when it becomes
	 * visible.
	 *
	 * Only views (and proxies in front of them) drive the loading: code
	 * reading the model directly sees it empty until then and should call
	 * EnsureLoaded() first.
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
		 * @param[in] load The function populating the model.
		 * @param[in] args The list of parameters to pass to the
		 * constructor of the base \em Model.
		 */
		template<typename... Args>
		explicit LazyRootMixin (std::function<void ()> load, Args&&... args)
		: Model { std::forward<Args> (args)... }
		, Load_ { std::move (load) }
		{
		}

		/** @brief Populates the model if it hasn't been populated yet.
		 */
		void EnsureLoaded ()
		{
			fetchMore ({});
		}

		bool hasChildren (const QModelIndex& parent) const override
		{
			return canFetchMore (parent) || Model::hasChildren (parent);
		}

		bool canFetchMore (const QModelIndex& parent) const override
		{
			return !parent.isValid () && !Loaded_;
		}

		void fetchMore (const QModelIndex& parent) override
		{
			if (!canFetchMore (parent))
				return;

			Loaded_ = true;
			Load_ ();
		}
	};
}
