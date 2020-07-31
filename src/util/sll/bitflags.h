/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>

namespace LC
{
namespace Util
{
	template<typename T>
	class BitFlags
	{
		static_assert (std::is_enum<T>::value, "The instantiating type should be a enumeration");

		using St_t = std::underlying_type_t<T>;
		St_t Storage_ = 0;
	public:
		BitFlags () = default;

		BitFlags (T t)
		: Storage_ { static_cast<St_t> (t) }
		{
		}

		explicit operator bool () const
		{
			return Storage_;
		}

		BitFlags& operator&= (BitFlags other)
		{
			Storage_ &= other.Storage_;
			return *this;
		}

		BitFlags& operator|= (BitFlags other)
		{
			Storage_ |= other.Storage_;
			return *this;
		}

		friend BitFlags operator& (BitFlags left, BitFlags right)
		{
			left &= right;
			return left;
		}

		friend BitFlags operator| (BitFlags left, BitFlags right)
		{
			left |= right;
			return left;
		}
	};
}
}

#define DECLARE_BIT_FLAGS(F) \
		inline LC::Util::BitFlags<F> operator& (F left, F right) \
		{ \
			return LC::Util::BitFlags<F> { left } & right; \
		} \
		inline LC::Util::BitFlags<F> operator| (F left, F right) \
		{ \
			return LC::Util::BitFlags<F> { left } | right; \
		}
