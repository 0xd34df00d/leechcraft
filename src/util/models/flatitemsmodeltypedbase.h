/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "flatitemsmodelbase.h"

namespace LC::Util
{
	template<typename T>
	class FlatItemsModelTypedBase : public FlatItemsModelBase
	{
	protected:
		QList<T> Items_;
	public:
		using FlatItemsModelBase::FlatItemsModelBase;

		void SetItems (QList<T> items)
		{
			beginResetModel ();
			Items_ = std::move (items);
			endResetModel ();
		}

		template<typename U>
			requires std::is_constructible_v<T, U&&>
		void SetItems (QList<U> items)
		{
			beginResetModel ();
			Items_.clear ();
			Items_.reserve (items.size ());
			for (auto&& item : items)
				Items_ << T { std::move (item) };
			endResetModel ();
		}

		const QList<T>& GetItems () const
		{
			return Items_;
		}

		void AddItem (const T& item)
		{
			beginInsertRows ({}, Items_.size (), Items_.size ());
			Items_.push_back (item);
			endInsertRows ();
		}

		void AddItems (const QList<T>& items)
		{
			if (items.isEmpty ())
				return;

			beginInsertRows ({}, Items_.size (), Items_.size () + items.size () - 1);
			Items_ += items;
			endInsertRows ();
		}

		void SetItem (int idx, const T& item)
		{
			Items_ [idx] = item;
			emit dataChanged (index (idx, 0),
					index (idx, columnCount ({}) - 1));
		}

		template<typename F>
		void EditItem (int idx, F&& editor)
		{
			std::invoke (std::forward<F> (editor), Items_ [idx]);
			emit dataChanged (index (idx, 0),
					index (idx, columnCount ({}) - 1));
		}

		void RemoveItem (int idx)
		{
			beginRemoveRows ({}, idx, idx);
			Items_.removeAt (idx);
			endRemoveRows ();
		}
	protected:
		int GetItemsCount () const override
		{
			return Items_.size ();
		}
	};
}
