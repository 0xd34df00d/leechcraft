/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <numeric>
#include <optional>

namespace LC
{
namespace Util
{
	template<typename T, typename SFINAE = void>
	struct InstanceMonadPlus
	{
		using UndefinedTag = void;
	};

	namespace detail
	{
		template<typename T>
		constexpr bool IsMonadPlusImpl (int, typename InstanceMonadPlus<T>::UndefinedTag* = nullptr)
		{
			return false;
		}

		template<typename T>
		constexpr bool IsMonadPlusImpl (float)
		{
			return true;
		}
	}

	template<typename T>
	constexpr bool IsMonadPlus ()
	{
		return detail::IsMonadPlusImpl<T> (0);
	}

	template<typename MP>
	MP Mzero ()
	{
		return InstanceMonadPlus<MP>::Mzero ();
	}

	const struct
	{
		template<typename MP>
		auto operator() (const MP& m1) const
		{
			return [m1] (const MP& m2) { return InstanceMonadPlus<MP>::Mplus (m1, m2); };
		}
	} Mplus {};

	template<typename MP>
	auto operator+ (const MP& m1, const MP& m2) -> decltype (Mplus (m1) (m2))
	{
		return Mplus (m1) (m2);
	}

	const struct
	{
		template<typename Vec>
		auto operator() (Vec&& vec) const
		{
			using std::begin;
			using std::end;
			using MP = typename Vec::value_type;
			return std::accumulate (begin (vec), end (vec), Mzero<MP> (), &operator+<MP>);
		}

		template<typename T>
		auto operator() (const std::initializer_list<T>& vec) const
		{
			using std::begin;
			using std::end;
			return std::accumulate (begin (vec), end (vec), Mzero<T> (), &operator+<T>);
		}
	} Msum {};

	template<typename T>
	struct InstanceMonadPlus<std::optional<T>>
	{
		static std::optional<T> Mzero ()
		{
			return {};
		}

		static std::optional<T> Mplus (const std::optional<T>& t1, const std::optional<T>& t2)
		{
			return t1 ? t1 : t2;
		}
	};
}
}
