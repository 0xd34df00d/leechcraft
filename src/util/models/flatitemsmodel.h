/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>
#include "flatitemsmodeltypedbase.h"

namespace LC::Util
{
	namespace detail
	{
		template<typename T, typename... Ts>
		QVariant GetFieldImpl (int idx, T& head, Ts&... tail)
		{
			if (!idx)
				return head;

			if constexpr (sizeof... (Ts))
				return GetFieldImpl (idx - 1, tail...);
			else
				throw std::runtime_error { "out of bounds tuple-like access" };
		}

		template<typename T>
		QVariant GetField (const T& item, int idx)
		{
			auto&& [...elems] = item;
			return GetFieldImpl (idx, elems...);
		}
	}

	template<typename T>
	class FlatItemsModel : public FlatItemsModelTypedBase<T>
	{
	public:
		using FlatItemsModelTypedBase<T>::FlatItemsModelTypedBase;
	protected:
		QVariant GetData (int row, int col, int role) const override
		{
			if (role == this->DataRole)
				return QVariant::fromValue (this->Items_.at (row));

			if (role != Qt::DisplayRole)
				return {};

			return detail::GetField (this->Items_.at (row), col);
		}
	};
}
