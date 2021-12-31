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
		QVector<T> Items_;
	public:
		using FlatItemsModelBase::FlatItemsModelBase;

		void SetItems (QVector<T> items)
		{
			beginResetModel ();
			Items_ = std::move (items);
			endResetModel ();
		}

		const QVector<T>& GetItems () const
		{
			return Items_;
		}

		void AddItem (const T& item)
		{
			beginInsertRows ({}, Items_.size (), Items_.size ());
			Items_.push_back (item);
			endInsertRows ();
		}

		void EditItem (int idx, const T& item)
		{
			Items_ [idx] = item;
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
