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
		struct Any
		{
			template<typename T>
			constexpr operator T ();
		};

		template<int>
		struct FC {};

		template<typename T>
		constexpr int GetFieldsCount ()
		{
			Any any;

			if constexpr (requires { T { any, any, any, any, any, any }; })
				return 6;
			else if constexpr (requires { T { any, any, any, any, any }; })
				return 5;
			else if constexpr (requires { T { any, any, any, any }; })
				return 4;
			else if constexpr (requires { T { any, any, any }; })
				return 3;
			else if constexpr (requires { T { any, any }; })
				return 2;
			else if constexpr (requires { T { any }; })
				return 1;
			else
				static_assert (std::is_same_v<T, struct Dummy>, "Don't know how to handle this type");
		}

		template<typename T>
		QVariant GetFieldImpl (const T& item, int idx, FC<1>)
		{
			auto [a0] = item;
			switch (idx)
			{
			case 0:
				return a0;
			default:
				return {};
			}
		}

		template<typename T>
		QVariant GetFieldImpl (const T& item, int idx, FC<2>)
		{
			auto [a0, a1] = item;
			switch (idx)
			{
			case 0:
				return a0;
			case 1:
				return a1;
			default:
				return {};
			}
		}

		template<typename T>
		QVariant GetFieldImpl (const T& item, int idx, FC<3>)
		{
			auto [a0, a1, a2] = item;
			switch (idx)
			{
			case 0:
				return a0;
			case 1:
				return a1;
			case 2:
				return a2;
			default:
				return {};
			}
		}

		template<typename T>
		QVariant GetFieldImpl (const T& item, int idx, FC<4>)
		{
			auto [a0, a1, a2, a3] = item;
			switch (idx)
			{
			case 0:
				return a0;
			case 1:
				return a1;
			case 2:
				return a2;
			case 3:
				return a3;
			default:
				return {};
			}
		}

		template<typename T>
		QVariant GetFieldImpl (const T& item, int idx, FC<5>)
		{
			auto [a0, a1, a2, a3, a4] = item;
			switch (idx)
			{
			case 0:
				return a0;
			case 1:
				return a1;
			case 2:
				return a2;
			case 3:
				return a3;
			case 4:
				return a4;
			default:
				return {};
			}
		}

		template<typename T>
		QVariant GetFieldImpl (const T& item, int idx, FC<6>)
		{
			auto [a0, a1, a2, a3, a4, a5] = item;
			switch (idx)
			{
			case 0:
				return a0;
			case 1:
				return a1;
			case 2:
				return a2;
			case 3:
				return a3;
			case 4:
				return a4;
			case 5:
				return a5;
			default:
				return {};
			}
		}

		template<typename T>
		QVariant GetField (const T& item, int idx)
		{
			return GetFieldImpl (item, idx, FC<GetFieldsCount<T> ()> {});
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
