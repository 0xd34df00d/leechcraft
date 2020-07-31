/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include "monadplus.h"

namespace LC
{
namespace Util
{
	template<typename T>
	using Lazy_t = std::function<T ()>;

	template<typename T>
	Lazy_t<T> MakeLazy (const T& t)
	{
		return [t] { return t; };
	}

	template<typename R, typename F>
	Lazy_t<R> MakeLazyF (const F& l)
	{
		return l;
	}

	template<typename T>
	struct InstanceMonadPlus<Lazy_t<T>, std::enable_if_t<IsMonadPlus<T> ()>>
	{
		static Lazy_t<T> Mzero ()
		{
			return [] { return Util::Mzero<T> (); };
		}

		static Lazy_t<T> Mplus (const Lazy_t<T>& t1, const Lazy_t<T>& t2)
		{
			return [=]
			{
				const auto rt1 = t1 ();
				return rt1 != Util::Mzero<T> () ? rt1 : t2 ();
			};
		}
	};
}
}
